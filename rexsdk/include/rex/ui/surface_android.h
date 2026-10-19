#pragma once

#include <android/native_window.h>

#include <rex/ui/surface.h>

struct SDL_Window;

namespace rex::ui {

class AndroidNativeWindowSurface final : public Surface {
 public:
  explicit AndroidNativeWindowSurface(ANativeWindow* window, SDL_Window* sdl_window)
      : window_(window), sdl_window_(sdl_window) {}

  TypeIndex GetType() const override { return kTypeIndex_AndroidNativeWindow; }
  ANativeWindow* window() const { return window_; }

 protected:
  bool GetSizeImpl(uint32_t& width_out, uint32_t& height_out) const override;

 private:
  ANativeWindow* window_;
  SDL_Window* sdl_window_;
};

}  // namespace rex::ui
