#include <rex/filesystem.h>

namespace rex::filesystem {

int OpenAndroidContentFileDescriptor(const std::string_view, const char*) {
  // SAF/JNI integration is supplied by the Android front end. Returning -1
  // keeps ordinary filesystem paths working until a content URI is selected.
  return -1;
}

}  // namespace rex::filesystem
