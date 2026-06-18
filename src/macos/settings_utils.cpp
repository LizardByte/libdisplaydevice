/**
 * @file src/macos/settings_utils.cpp
 * @brief Definitions for macOS settings utility functions.
 */
// header include
#include "display_device/macos/settings_utils.h"

namespace display_device::mac_utils {
  StringSet flattenTopology(const MacActiveTopology &topology) {
    StringSet flattened_topology;
    for (const auto &group : topology) {
      for (const auto &device_id : group) {
        flattened_topology.insert(device_id);
      }
    }

    return flattened_topology;
  }

  void noopGuard() {
    // Intentionally empty guard callback.
  }
}  // namespace display_device::mac_utils
