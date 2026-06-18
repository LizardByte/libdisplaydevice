/**
 * @file src/macos/mac_display_device_primary.cpp
 * @brief Definitions for primary display related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

namespace display_device {
  bool MacDisplayDevice::isPrimary(const std::string &device_id) const {
    static_cast<void>(device_id);
    return false;
  }

  bool MacDisplayDevice::setAsPrimary(const std::string &device_id) {
    static_cast<void>(device_id);
    return false;
  }
}  // namespace display_device
