#pragma once

// system includes
#include <tuple>

// local includes
#include "types.h"
#include "windisplaydeviceinterface.h"

/**
 * @brief Shared "utility-level" code for Settings.
 */
namespace display_device::win_utils {
  /**
   * @brief Get all the device its in the topology.
   * @param topology Topology to be "flattened".
   * @return Device ids found in the topology.
   *
   * EXAMPLES:
   * ```cpp
   * const ActiveTopology topology { { "DeviceId1", "DeviceId2" }, { "DeviceId3" } };
   * const auto device_ids { flattenTopology(topology) };
   * ```
   */
  std::set<std::string>
  flattenTopology(const ActiveTopology &topology);

  /**
   * @brief Compute the new intial state from arbitrary data.
   * @param prev_state Previous initial state if available.
   * @param topology_before_changes Topology before any changes were made.
   * @param devices Currently available device list.
   * @return New initial state that should be used.
   *
   * EXAMPLES:
   * ```cpp
   * const SingleDisplayConfigState::Initial prev_state { { { "DeviceId1", "DeviceId2" }, { "DeviceId3" } } };
   * const ActiveTopology topology_before_changes { { "DeviceId3" } };
   * const EnumeratedDeviceList devices { ... };
   *
   * const auto new_initial_state { computeInitialState(prev_state, topology_before_changes, devices) };
   * ```
   */
  std::optional<SingleDisplayConfigState::Initial>
  computeInitialState(const std::optional<SingleDisplayConfigState::Initial> &prev_state,
    const ActiveTopology &topology_before_changes,
    const EnumeratedDeviceList &devices);

  /**
   * @brief Strip the initial state of non-existing devices.
   * @param initial_state State to be stripped.
   * @param devices Currently available device list.
   * @return Stripped initial state.
   */
  std::optional<SingleDisplayConfigState::Initial>
  stripInitialState(const SingleDisplayConfigState::Initial &initial_state, const EnumeratedDeviceList &devices);

  /**
   * @brief Compute new topology from arbitrary.
   * @param device_prep Specify how to to compute the new topology.
   * @param configuring_primary_devices Specify whether the `device_to_configure` was unspecified (primary device was selected).
   * @param device_to_configure Main device to be configured.
   * @param additional_devices_to_configure Additional devices that belong to the same group as `device_to_configure`.
   * @param initial_topology The initial topology from `computeInitialState(...)`.
   * @return New topology that should be set.
   */
  ActiveTopology
  computeNewTopology(SingleDisplayConfiguration::DevicePreparation device_prep,
    bool configuring_primary_devices,
    const std::string &device_to_configure,
    const std::set<std::string> &additional_devices_to_configure,
    const ActiveTopology &initial_topology);

  /**
   * @brief Compute new topology + metadata from config settings and initial state.
   * @param device_prep Specify how to to compute the new topology.
   * @param device_id Specify which device whould be used for computation (can be empty if primary device should be used).
   * @param initial_state The initial topology from `computeInitialState(...)` or `stripInitialState(...)` (or both).
   * @return A tuple of (new_topology, device_to_configure, addotional_devices_to_configure).
   */
  std::tuple<ActiveTopology, std::string, std::set<std::string>>
  computeNewTopologyAndMetadata(SingleDisplayConfiguration::DevicePreparation device_prep,
    const std::string &device_id,
    const SingleDisplayConfigState::Initial &initial_state);

  /**
   * @brief Make guard function for the topology.
   * @param win_dd Interface for interacting with the OS.
   * @param topology Topology to be used when making a guard.
   * @return Function that once called will try to revert topology to the provided one.
   *
   * EXAMPLES:
   * ```cpp
   * WinDisplayDeviceInterface* iface = getIface(...);
   * boost::scope::scope_exit guard { topologyGuardFn(*iface, iface->getCurrentTopology()) };
   * ```
   */
  DdGuardFn
  topologyGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology);

  /**
   * @brief Make guard function for the display modes.
   * @param win_dd Interface for interacting with the OS.
   * @param topology Topology to be used when making a guard.
   * @return Function that once called will try to revert display modes to the ones that were initially set for the provided topology.
   *
   * EXAMPLES:
   * ```cpp
   * WinDisplayDeviceInterface* iface = getIface(...);
   * boost::scope::scope_exit guard { modeGuardFn(*iface, iface->getCurrentTopology()) };
   * ```
   */
  DdGuardFn
  modeGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology);

  /**
   * @brief Make guard function for the primary display.
   * @param win_dd Interface for interacting with the OS.
   * @param topology Topology to be used when making a guard.
   * @return Function that once called will try to revert primary display to the one that was initially set for the provided topology.
   *
   * EXAMPLES:
   * ```cpp
   * WinDisplayDeviceInterface* iface = getIface(...);
   * boost::scope::scope_exit guard { primaryGuardFn(*iface, iface->getCurrentTopology()) };
   * ```
   */
  DdGuardFn
  primaryGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology);

  /**
   * @brief Make guard function for the HDR states.
   * @param win_dd Interface for interacting with the OS.
   * @param topology Topology to be used when making a guard.
   * @return Function that once called will try to revert HDR states to the ones that were initially set for the provided topology.
   *
   * EXAMPLES:
   * ```cpp
   * WinDisplayDeviceInterface* iface = getIface(...);
   * boost::scope::scope_exit guard { hdrStateGuardFn(*iface, iface->getCurrentTopology()) };
   * ```
   */
  DdGuardFn
  hdrStateGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology);
}  // namespace display_device::win_utils
