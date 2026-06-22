/**
 * @file src/macos/mac_display_device_hdr.cpp
 * @brief Definitions for HDR related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

// system includes
#include <algorithm>

namespace display_device {
  MacHdrStateMap MacDisplayDevice::getCurrentHdrStates(const StringSet &device_ids) const {
    MacHdrStateMap states;
    for (const auto &device_id : device_ids) {
      states[device_id] = std::nullopt;
    }

    return states;
  }

  bool MacDisplayDevice::setHdrStates(const MacHdrStateMap &states) {
    return std::ranges::all_of(states, [](const auto &entry) {
      return !entry.second;
    });
  }
}  // namespace display_device
