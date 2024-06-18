// local includes
#include "displaydevice/noopaudiocontext.h"

namespace display_device {
  bool
  NoopAudioContext::capture() {
    m_is_captured = true;
    return true;
  }

  bool
  NoopAudioContext::isCaptured() const {
    return m_is_captured;
  }

  void
  NoopAudioContext::release() {
    m_is_captured = false;
  }
}  // namespace display_device
