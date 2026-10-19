/**
 * Android fallback for the host fiber abstraction.
 *
 * NDK r26 does not expose ucontext. This keeps the runtime linkable while
 * guest fiber scheduling remains disabled until an Android context backend is
 * added.
 */
#include <rex/thread/fiber.h>

namespace rex::thread {

thread_local Fiber* Fiber::tls_current_ = nullptr;

Fiber* Fiber::ConvertCurrentThread() {
  auto* fiber = new Fiber();
  tls_current_ = fiber;
  return fiber;
}

Fiber* Fiber::Create(size_t, void (*)(void*), void*) {
  return nullptr;
}

void Fiber::SwitchTo(Fiber* target) {
  tls_current_ = target;
}

void Fiber::Destroy() {
  if (tls_current_ == this) {
    tls_current_ = nullptr;
  }
  delete this;
}

}  // namespace rex::thread
