/**
 * @file src/macos/mac_display_device_primary.cpp
 * @brief Definitions for primary display related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

namespace display_device {
  bool MacDisplayDevice::isPrimary(const std::string &device_id) const {
    const auto display_id {getDisplayId(device_id, MacQueryType::Active)};
    return display_id && m_m_api->isMainDisplay(*display_id);
  }

  bool MacDisplayDevice::setAsPrimary(const std::string &device_id) {
    static_cast<void>(device_id);
    return false;
  }
}  // namespace display_device
