/**
 * @file src/macos/mac_display_device_hdr.cpp
 * @brief Definitions for HDR related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

namespace display_device {
  MacHdrStateMap MacDisplayDevice::getCurrentHdrStates(const StringSet &device_ids) const {
    MacHdrStateMap states;
    for (const auto &device_id : device_ids) {
      states[device_id] = std::nullopt;
    }

    return states;
  }

  bool MacDisplayDevice::setHdrStates(const MacHdrStateMap &states) {
    for (const auto &[device_id, state] : states) {
      static_cast<void>(device_id);
      if (state) {
        return false;
      }
    }

    return true;
  }
}  // namespace display_device
