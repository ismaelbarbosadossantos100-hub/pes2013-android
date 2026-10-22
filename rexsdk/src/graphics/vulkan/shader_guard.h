// shader_guard.h - PES13 recomp (Mali / low-RAM Android)
// 1) Logs hash+size of every runtime GLSL->SPIR-V compile BEFORE entering glslang.
// 2) Watchdog timeout: compile runs on a worker; if it exceeds the limit the caller
//    gets "false" (existing callers already just WARN and skip) and the GPU thread
//    is never blocked for minutes again.
// 3) Disk SPIR-V cache (<hash>.spv) so each shader is compiled once, ever.
// 4) Blacklist: <hash>.pending marker survives a force-close/kill during compile;
//    on next launch a leftover .pending or .bad file => skip instantly.
// Nothing is ever deleted by this code except its own .pending marker.
#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <rex/logging.h>

namespace rex::graphics::vulkan::shader_guard {

using CompileFn = std::function<bool(std::vector<uint32_t>&, std::string&)>;

inline uint64_t HashSource(std::string_view s) {  // FNV-1a 64
  uint64_t h = 1469598103934665603ull;
  for (unsigned char c : s) { h ^= c; h *= 1099511628211ull; }
  return h;
}

inline std::filesystem::path& CacheDir() {
  static std::filesystem::path dir;
  return dir;
}

inline void SetCacheDir(const std::filesystem::path& dir) {
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (!ec) CacheDir() = dir;
}

namespace detail {
constexpr uint32_t kCacheVersion = 1;
inline std::filesystem::path P(uint64_t h, const char* ext) {
  char n[48];
  std::snprintf(n, sizeof(n), "%016llx_v%u.%s", (unsigned long long)h, kCacheVersion, ext);
  return CacheDir() / n;
}
inline bool Exists(const std::filesystem::path& p) {
  std::error_code ec; return !CacheDir().empty() && std::filesystem::exists(p, ec);
}
inline void Touch(const std::filesystem::path& p, const char* text = "") {
  if (CacheDir().empty()) return;
  std::ofstream f(p, std::ios::binary); if (f) f << text;
}
inline bool Load(uint64_t h, std::vector<uint32_t>& out) {
  if (CacheDir().empty()) return false;
  std::ifstream f(P(h, "spv"), std::ios::binary | std::ios::ate);
  if (!f) return false;
  std::streamsize n = f.tellg();
  if (n < 20 || n % 4) return false;
  out.resize(size_t(n) / 4);
  f.seekg(0);
  if (!f.read(reinterpret_cast<char*>(out.data()), n)) { out.clear(); return false; }
  if (out[0] != 0x07230203u) { out.clear(); return false; }  // SPIR-V magic
  return true;
}
inline void Store(uint64_t h, const std::vector<uint32_t>& spv) {
  if (CacheDir().empty() || spv.empty()) return;
  auto p = P(h, "spv"); auto tmp = p; tmp += ".tmp";
  { std::ofstream f(tmp, std::ios::binary);
    if (!f) return;
    f.write(reinterpret_cast<const char*>(spv.data()), std::streamsize(spv.size() * 4)); }
  std::error_code ec; std::filesystem::rename(tmp, p, ec);
}
struct Shared {
  std::mutex m; std::condition_variable cv;
  bool done = false, ok = false;
  std::vector<uint32_t> spirv; std::string error;
};
}  // namespace detail

inline bool CompileGuarded(const char* name, std::string_view source, CompileFn fn,
                           std::vector<uint32_t>& spirv_out, std::string& error_out,
                           uint32_t timeout_ms = 20000) {
  const uint64_t h = HashSource(source);
  REXGPU_INFO("[shader-guard] BEGIN {} hash={:016X} bytes={} timeout={}ms", name, h,
              source.size(), timeout_ms);

  if (detail::Load(h, spirv_out)) {
    REXGPU_INFO("[shader-guard] CACHE-HIT {} hash={:016X} words={}", name, h, spirv_out.size());
    return true;
  }
  if (detail::Exists(detail::P(h, "bad"))) {
    error_out = "shader blacklisted (previous timeout)";
    REXGPU_WARN("[shader-guard] SKIP {} hash={:016X}: blacklisted (.bad)", name, h);
    return false;
  }
  if (detail::Exists(detail::P(h, "pending"))) {
    // previous run was killed/hung while compiling this exact shader
    detail::Touch(detail::P(h, "bad"), "hang-or-kill-during-compile");
    error_out = "shader hung/killed previous run";
    REXGPU_WARN("[shader-guard] SKIP {} hash={:016X}: leftover .pending => blacklisted", name, h);
    return false;
  }

  detail::Touch(detail::P(h, "pending"));
  auto sh = std::make_shared<detail::Shared>();
  auto t0 = std::chrono::steady_clock::now();
  std::thread([sh, fn = std::move(fn)]() {
    std::vector<uint32_t> spv; std::string err;
    bool ok = fn(spv, err);
    std::lock_guard<std::mutex> lk(sh->m);
    sh->ok = ok; sh->spirv = std::move(spv); sh->error = std::move(err); sh->done = true;
    sh->cv.notify_all();
  }).detach();  // detached: owns shared_ptr, safe if we stop waiting

  std::unique_lock<std::mutex> lk(sh->m);
  bool finished = sh->cv.wait_for(lk, std::chrono::milliseconds(timeout_ms),
                                  [&] { return sh->done; });
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - t0).count();
  if (!finished) {
    detail::Touch(detail::P(h, "bad"), "timeout");
    error_out = "shader compile timeout";
    REXGPU_ERROR("[shader-guard] TIMEOUT {} hash={:016X} after {}ms -> skipped + blacklisted",
                 name, h, ms);
    return false;  // .pending kept on purpose; worker keeps running harmlessly
  }
  std::error_code ec;
  std::filesystem::remove(detail::P(h, "pending"), ec);  // our own marker only
  if (!sh->ok) {
    error_out = sh->error;
    REXGPU_WARN("[shader-guard] FAIL {} hash={:016X} {}ms: {}", name, h, ms, sh->error);
    return false;
  }
  spirv_out = std::move(sh->spirv);
  detail::Store(h, spirv_out);
  REXGPU_INFO("[shader-guard] OK {} hash={:016X} {}ms words={}", name, h, ms, spirv_out.size());
  return true;
}

}  // namespace rex::graphics::vulkan::shader_guard
