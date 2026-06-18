/**
 * @file src/macos/include/display_device/macos/settings_utils.h
 * @brief Declarations for macOS settings utility functions.
 */
#pragma once

// local includes
#include "types.h"

/**
 * @brief Shared utility-level code for macOS settings.
 */
namespace display_device::mac_utils {
  /**
   * @brief Get all the device ids in the topology.
   * @param topology Topology to flatten.
   * @return Device ids found in the topology.
   */
  [[nodiscard]] StringSet flattenTopology(const MacActiveTopology &topology);

  /**
   * @brief Function that does nothing.
   */
  void noopGuard();
}  // namespace display_device::mac_utils
