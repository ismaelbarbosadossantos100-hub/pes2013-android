// Lightweight progress counters used by the [hb] heartbeat log (rex_app.cpp) to
// tell whether the guest/GPU make progress or the app is idle/stuck.
#pragma once

#include <atomic>
#include <cstdint>

namespace rex::diag {

struct Counters {
  std::atomic<uint64_t> packets{0};     // PM4 packets executed by the GPU command processor
  std::atomic<uint64_t> draws_ok{0};    // draws accepted by the backend
  std::atomic<uint64_t> draws_fail{0};  // draws rejected by the backend
  std::atomic<uint64_t> gpu_swaps{0};   // IssueSwap calls (frames handed to the presenter)
  std::atomic<uint64_t> vd_swaps{0};    // guest VdSwap calls
  std::atomic<uint64_t> interrupts{0};  // graphics interrupt callbacks dispatched to the guest
  std::atomic<uint64_t> file_opens{0};  // NtCreateFile/NtOpenFile calls
  std::atomic<uint64_t> gpu_idle_iters{0};  // idle-wait loop iterations of the GPU worker
};

inline Counters& counters() {
  static Counters c;
  return c;
}

}  // namespace rex::diag
