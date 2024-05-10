// class header include
#include "displaydevice/windows/windisplaydevice.h"

// system includes
#include <stdexcept>

namespace display_device {
  WinDisplayDevice::WinDisplayDevice(std::shared_ptr<WinApiLayerInterface> w_api):
      m_w_api { std::move(w_api) } {
    if (!m_w_api) {
      throw std::logic_error { "Nullptr provided for WinApiLayerInterface in WinDisplayDevice!" };
    }
  }
}  // namespace display_device
