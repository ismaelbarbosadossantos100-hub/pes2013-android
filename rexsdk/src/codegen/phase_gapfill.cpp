/**
 * @file        codegen/phase_gapfill.cpp
 * @brief       GapFill phase: find uncovered code regions and register them as functions
 *
 * @copyright   Copyright (c) 2026 Tom Clay <tomc@tctechstuff.com>
 *              All rights reserved.
 *
 * @license     BSD 3-Clause License
 *              See LICENSE file in the project root for full license text.
 *
 * PATCH v2: carve uncovered tails after noreturn calls
 * =====================================================
 * PROBLEM: When a code segment's START falls inside an already-known function
 * (e.g. because the known function ends with a noreturn call like
 * `bl _CxxThrowException` and the next function is placed immediately after
 * with no blr/padding), the old code discarded the entire segment, losing any
 * functions that follow the known function's end.
 *
 * FIX: If segment.start is inside a known function AND that function's end()
 * falls strictly inside the segment, AND nothing is already registered at that
 * boundary, carve out only the uncovered tail starting at coveredEnd.
 *
 * ADDITIONAL FIX (v2 vs v1): Before calling addFunction, scan for any already-
 * registered entry point between segStart and segment.end. If one exists,
 * truncate the new GAP_FILL at that point so we never create a range that
 * overlaps an existing function's declared start.
 */

#include "ppc/instruction.h"

#include <unordered_set>

#include <rex/codegen/phases.h>
#include "phase_helpers.h"

#include <rex/logging.h>

#include "codegen_logging.h"
#include <rex/memory/utils.h>

#include <ppc.h>

using rex::codegen::ppc::decode_instruction;
using rex::codegen::ppc::Opcode;
using rex::memory::load_and_swap;

namespace rex::codegen {

namespace {

//=============================================================================
// GapFill to register uncovered code regions
//=============================================================================

// Split a code region into function segments based on terminators (blr, tail calls).
std::vector<CodeRegion> splitRegionOnTerminators(
    const CodeRegion& region, const BinaryView& binary,
    const std::unordered_set<uint32_t>& knownCallables) {
  std::vector<CodeRegion> segments;
  uint32_t segmentStart = region.start;

  for (uint32_t addr = region.start; addr < region.end; addr += 4) {
    const uint8_t* data = binary.translate(addr);
    if (!data)
      break;

    uint32_t raw = load_and_swap<uint32_t>(data);
    auto decoded = decode_instruction(addr, raw);
    bool shouldSplit = false;
    const char* reason = nullptr;

    // Check for terminators
    if (decoded.is_return()) {
      shouldSplit = true;
      reason = "blr";
    } else if (decoded.opcode == Opcode::b && decoded.branch_target.has_value()) {
      uint32_t target = decoded.branch_target.value();
      // Don't split on tail recursion (branch to own segment start)
      if (target != segmentStart && knownCallables.contains(target)) {
        shouldSplit = true;
        reason = "tail call";
      }
    }

    if (shouldSplit) {
      uint32_t segmentEnd = addr + 4;
      if (segmentEnd > segmentStart) {
        segments.push_back({segmentStart, segmentEnd});
        REXCODEGEN_TRACE("GapFill: split segment 0x{:08X}-0x{:08X} ({} at 0x{:08X})", segmentStart,
                         segmentEnd, reason, addr);
      }
      segmentStart = segmentEnd;
    }
  }

  // Handle remaining code after last terminator
  if (segmentStart < region.end) {
    segments.push_back({segmentStart, region.end});
  }

  return segments;
}

// Check if address looks like exception handler data (handler ptr + rdata ptr)
bool looksLikeExceptionData(const BinaryView& binary, const FunctionGraph& graph, uint32_t addr) {
  const uint8_t* data = binary.translate(addr);
  if (!data)
    return false;

  // Exception handler data pattern:
  // [addr+0]: pointer to __C_specific_handler (entry point)
  // [addr+4]: pointer to scope table in .rdata
  uint32_t firstDword = load_and_swap<uint32_t>(data);
  uint32_t secondDword = load_and_swap<uint32_t>(data + 4);

  // Check if first dword is a known entry point (like __C_specific_handler)
  if (!graph.isEntryPoint(firstDword)) {
    return false;
  }

  // Check if second dword points to .rdata section
  auto* rdataSection = binary.findSectionByName(".rdata");
  if (!rdataSection)
    return false;

  uint32_t rdataStart = rdataSection->baseAddress;
  uint32_t rdataEnd = rdataStart + rdataSection->size;

  if (secondDword >= rdataStart && secondDword < rdataEnd) {
    REXCODEGEN_TRACE(
        "GapFill: 0x{:08X} looks like exception data (handler=0x{:08X}, scope=0x{:08X}), skipping",
        addr, firstDword, secondDword);
    return true;
  }

  return false;
}

// Find the first registered entry point in the half-open range (start, end).
// Returns 0 if none found.
// Invariant: start and end must be 4-byte aligned; start < end.
uint32_t findFirstEntryPointAfter(const FunctionGraph& graph, uint32_t start, uint32_t end) {
  // functionsByBase_ is a sorted map; use it directly if accessible.
  // We iterate graph.functions() — O(f) but only called for GAP_FILL candidates.
  for (const auto& [addr, node] : graph.functions()) {
    if (addr > start && addr < end) {
      return addr;
    }
  }
  return 0;
}

void gapFillCodeRegions(CodegenContext& ctx) {
  REXCODEGEN_TRACE("Analyze: checking for uncovered code regions...");

  auto& graph = ctx.graph;
  auto& binary = ctx.binary();
  auto& scan = ctx.scan;

  // Build set of known callables for tail call detection
  std::unordered_set<uint32_t> knownCallables;
  for (const auto& [addr, node] : graph.functions()) {
    knownCallables.insert(addr);
  }

  size_t gapsFound = 0;
  size_t segmentsCreated = 0;
  size_t carvedTails = 0;

  for (const auto& region : scan.codeRegions) {
    // Split region on terminators (blr, tail calls), then check each segment
    auto segments = splitRegionOnTerminators(region, binary, knownCallables);

    for (const auto& segment : segments) {
      // Skip if this segment's start is already a registered function entry.
      // NOTE: we still need to check if a function at segment.start ends before
      // segment.end — handled by the containingFunc branch below.
      if (graph.isEntryPoint(segment.start)) {
        // The entry is known; check if its declared end leaves an uncovered tail.
        // This covers the edge case: isEntryPoint==true AND function ends early.
        const FunctionNode* existingFunc = graph.getFunction(segment.start);
        if (!existingFunc) {
          continue;  // Shouldn't happen, but be safe
        }
        const uint32_t declaredEnd = existingFunc->end();
        if (declaredEnd >= segment.end) {
          continue;  // Entire segment covered
        }
        // Tail exists: treat declaredEnd as segStart for the uncovered portion.
        uint32_t segStart = declaredEnd;
        if (graph.isEntryPoint(segStart) || graph.getFunctionContaining(segStart)) {
          continue;  // Already covered at boundary
        }
        // Clamp to next known entry point to avoid overlapping it
        uint32_t nextEntry = findFirstEntryPointAfter(graph, segStart, segment.end);
        uint32_t segEnd = (nextEntry != 0) ? nextEntry : segment.end;
        if (segEnd <= segStart || (segEnd - segStart) % 4 != 0) {
          continue;
        }
        if (looksLikeExceptionData(binary, graph, segStart))
          continue;
        uint32_t segmentSize = segEnd - segStart;
        graph.addFunction(segStart, segmentSize, FunctionAuthority::GAP_FILL, false);
        REXCODEGEN_TRACE(
            "GapFill: segment 0x{:08X}-0x{:08X} starts at known entry sub_{:08X} (ends "
            "0x{:08X}); carving tail 0x{:08X}-0x{:08X} ({} bytes)",
            segment.start, segment.end, segment.start, declaredEnd, segStart, segEnd,
            segmentSize);
        segmentsCreated++;
        carvedTails++;
        continue;
      }

      // The splitter only cuts on `blr` and on `b` to a known callable. A function that
      // ends in a call that never returns (MSVC's _CxxThrowException after `bl`, an
      // abort/trap helper, ...) is followed *directly* by the next function without any
      // such terminator, so one segment can start inside an already-known function and
      // continue into one or more functions that nothing has registered yet. Those are
      // typically leaf functions without .pdata (static initializers, thunks), reachable
      // only through data tables or tail calls.
      //
      // Previously the whole segment was dropped when its start was covered, which lost
      // every function after the covered prefix. Now only the covered prefix is dropped.
      uint32_t segStart = segment.start;
      if (const FunctionNode* containingFunc = graph.getFunctionContaining(segStart)) {
        const uint32_t coveredEnd = containingFunc->end();
        // Require a clean boundary: the covering function must end strictly inside this
        // segment, and nothing may be registered/covering at that boundary already.
        // (Holes inside a function's own blocks are NOT turned into functions.)
        if (coveredEnd <= segStart || coveredEnd >= segment.end)
          continue;
        if (graph.isEntryPoint(coveredEnd) || graph.getFunctionContaining(coveredEnd))
          continue;
        REXCODEGEN_TRACE(
            "GapFill: segment 0x{:08X}-0x{:08X} starts inside sub_{:08X} (ends 0x{:08X}); "
            "carving uncovered tail",
            segment.start, segment.end, containingFunc->base(), coveredEnd);
        segStart = coveredEnd;
        carvedTails++;
      }

      // Skip if this looks like exception handler data (handler ptr + rdata ptr)
      if (looksLikeExceptionData(binary, graph, segStart))
        continue;

      // Clamp to the next known entry point to avoid creating a GAP_FILL that
      // overlaps another function's declared start.
      uint32_t nextEntry = findFirstEntryPointAfter(graph, segStart, segment.end);
      uint32_t segEnd = (nextEntry != 0) ? nextEntry : segment.end;

      if (segEnd <= segStart) {
        continue;  // Zero-size or inverted range
      }
      if ((segEnd - segStart) % 4 != 0) {
        // Alignment invariant violated; skip rather than generate misaligned function
        REXCODEGEN_TRACE("GapFill: skipping 0x{:08X} (size 0x{:X} not 4-byte aligned)",
                         segStart, segEnd - segStart);
        continue;
      }

      uint32_t segmentSize = segEnd - segStart;
      graph.addFunction(segStart, segmentSize, FunctionAuthority::GAP_FILL, false);

      REXCODEGEN_TRACE("GapFill: registered sub_{:08X} (0x{:08X}-0x{:08X}, {} bytes)", segStart,
                       segStart, segEnd, segmentSize);
      segmentsCreated++;
    }

    gapsFound++;
  }

  if (segmentsCreated > 0) {
    REXCODEGEN_TRACE("Analyze: registered {} gap functions from {} regions ({} carved after a "
                     "covered prefix)",
                     segmentsCreated, gapsFound, carvedTails);
  } else {
    REXCODEGEN_TRACE("Analyze: no uncovered regions found");
  }
}

//=============================================================================
// Cleanup absorbed GAP_FILL functions
//=============================================================================

void cleanupAbsorbedGapFills(CodegenContext& ctx) {
  auto& graph = ctx.graph;
  std::vector<uint32_t> toRemove;

  for (const auto& [addr, node] : graph.functions()) {
    if (node->authority() != FunctionAuthority::GAP_FILL)
      continue;

    for (const auto& [otherAddr, otherNode] : graph.functions()) {
      if (otherAddr == addr)
        continue;
      if (!otherNode->containsAddress(addr))
        continue;

      // This GAP_FILL is inside another function's blocks
      if (otherNode->authority() != FunctionAuthority::GAP_FILL) {
        // Absorbed by higher authority - remove
        toRemove.push_back(addr);
        break;
      } else if (otherAddr < addr) {
        // Both GAP_FILL, other has lower address - it survives
        toRemove.push_back(addr);
        break;
      }
    }
  }

  for (uint32_t addr : toRemove) {
    graph.removeFunction(addr);
  }

  if (!toRemove.empty()) {
    REXCODEGEN_TRACE("Analyze: removed {} absorbed GAP_FILL functions", toRemove.size());
  }
}

}  // anonymous namespace

namespace phases {

VoidResult GapFill(CodegenContext& ctx, ProgressReporter* reporter) {
  (void)reporter;
  gapFillCodeRegions(ctx);

  // Discover blocks for gap-filled functions
  auto known = buildKnownFunctions(ctx.graph, /*excludeGapFill=*/true);
  size_t discovered = discoverPendingFunctions(ctx, known);
  REXCODEGEN_TRACE("Analyze: discovered blocks for {} gap-filled functions", discovered);

  cleanupAbsorbedGapFills(ctx);

  return Ok();
}

}  // namespace phases

}  // namespace rex::codegen
