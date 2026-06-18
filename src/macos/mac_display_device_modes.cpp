/**
 * @file src/macos/mac_display_device_modes.cpp
 * @brief Definitions for display mode related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

namespace display_device {
  MacDeviceDisplayModeMap MacDisplayDevice::getCurrentDisplayModes(const StringSet &device_ids) const {
    static_cast<void>(device_ids);
    return {};
  }

  bool MacDisplayDevice::setDisplayModes(const MacDeviceDisplayModeMap &modes) {
    static_cast<void>(modes);
    return false;
  }
}  // namespace display_device
