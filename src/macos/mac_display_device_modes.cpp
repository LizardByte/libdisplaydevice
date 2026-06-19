/**
 * @file src/macos/mac_display_device_modes.cpp
 * @brief Definitions for display mode related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

// local includes
#include "display_device/logging.h"

namespace display_device {
  MacDeviceDisplayModeMap MacDisplayDevice::getCurrentDisplayModes(const StringSet &device_ids) const {
    if (device_ids.empty()) {
      DD_LOG(error) << "Device id set is empty!";
      return {};
    }

    MacDeviceDisplayModeMap current_modes;
    for (const auto &device_id : device_ids) {
      const auto display_id {getDisplayId(device_id, MacQueryType::Active)};
      if (!display_id) {
        DD_LOG(error) << "Failed to find active macOS display for " << device_id << "!";
        return {};
      }

      const auto current_mode {m_m_api->getCurrentDisplayMode(*display_id)};
      if (!current_mode) {
        DD_LOG(error) << "Failed to get current macOS display mode for " << device_id << "!";
        return {};
      }

      current_modes[device_id] = *current_mode;
    }

    return current_modes;
  }

  bool MacDisplayDevice::setDisplayModes(const MacDeviceDisplayModeMap &modes) {
    static_cast<void>(modes);
    return false;
  }
}  // namespace display_device
