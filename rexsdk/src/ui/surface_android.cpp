#include <rex/ui/surface_android.h>

#include <SDL3/SDL.h>

namespace rex::ui {

bool AndroidNativeWindowSurface::GetSizeImpl(uint32_t& width_out, uint32_t& height_out) const {
  if (window_) {
    const int width = ANativeWindow_getWidth(window_);
    const int height = ANativeWindow_getHeight(window_);
    if (width > 0 && height > 0) {
      width_out = static_cast<uint32_t>(width);
      height_out = static_cast<uint32_t>(height);
      return true;
    }
  }
  if (sdl_window_) {
    int width = 0;
    int height = 0;
    if (SDL_GetWindowSizeInPixels(sdl_window_, &width, &height) && width > 0 && height > 0) {
      width_out = static_cast<uint32_t>(width);
      height_out = static_cast<uint32_t>(height);
      return true;
    }
  }
  return false;
}

}  // namespace rex::ui
