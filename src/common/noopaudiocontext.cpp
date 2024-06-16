// local includes
#include "displaydevice/noopaudiocontext.h"

namespace display_device {
  bool
  NoopAudioContext::capture(const std::vector<std::string> &) {
    return true;
  }
  
  void
  NoopAudioContext::release(const std::vector<std::string> &) {
  }
}  // namespace display_device
