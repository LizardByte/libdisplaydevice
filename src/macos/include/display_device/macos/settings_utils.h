/**
 * @file src/macos/include/display_device/macos/settings_utils.h
 * @brief Declarations for macOS settings utility functions.
 */
#pragma once

// local includes
#include "mac_display_device_interface.h"
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
   * @brief Get one primary device from the provided topology.
   * @param mac_dd Interface for interacting with the OS.
   * @param topology Topology to search.
   * @return Primary device id, or an empty string if none can be found.
   */
  [[nodiscard]] std::string getPrimaryDevice(const MacDisplayDeviceInterface &mac_dd, const MacActiveTopology &topology);

  /**
   * @brief Compute the initial state that should be used for future reverts.
   * @param prev_state Previous initial state if one was persisted.
   * @param topology_before_changes Current topology before applying a new configuration.
   * @param devices Currently available devices.
   * @return Initial state, or empty optional if the state cannot be computed.
   */
  [[nodiscard]] std::optional<MacSingleDisplayConfigState::Initial> computeInitialState(
    const std::optional<MacSingleDisplayConfigState::Initial> &prev_state,
    const MacActiveTopology &topology_before_changes,
    const EnumeratedDeviceList &devices
  );

  /**
   * @brief Remove unavailable devices from a stored initial state.
   * @param initial_state State to strip.
   * @param devices Currently available devices.
   * @return Stripped state, or empty optional if no usable state remains.
   */
  [[nodiscard]] std::optional<MacSingleDisplayConfigState::Initial> stripInitialState(
    const MacSingleDisplayConfigState::Initial &initial_state,
    const EnumeratedDeviceList &devices
  );

  /**
   * @brief Compute display modes requested by a single-display configuration.
   * @param resolution Optional resolution override.
   * @param refresh_rate Optional refresh-rate override.
   * @param configuring_primary_devices True when an empty device id selected primary devices.
   * @param device_to_configure Main device being configured.
   * @param additional_devices_to_configure Additional devices mirrored with the main device.
   * @param original_modes Current or persisted display modes used as the base.
   * @return New mode map with requested changes applied.
   */
  [[nodiscard]] MacDeviceDisplayModeMap computeNewDisplayModes(
    const std::optional<Resolution> &resolution,
    const std::optional<FloatingPoint> &refresh_rate,
    bool configuring_primary_devices,
    const std::string &device_to_configure,
    const StringSet &additional_devices_to_configure,
    const MacDeviceDisplayModeMap &original_modes
  );

  /**
   * @brief Make a guard function for display modes.
   * @param mac_dd Interface for interacting with the OS.
   * @param modes Display modes to restore when the guard runs.
   * @return Function that tries to restore the provided modes.
   */
  [[nodiscard]] MacDdGuardFn modeGuardFn(MacDisplayDeviceInterface &mac_dd, const MacDeviceDisplayModeMap &modes);

  /**
   * @brief Function that does nothing.
   */
  void noopGuard();
}  // namespace display_device::mac_utils
