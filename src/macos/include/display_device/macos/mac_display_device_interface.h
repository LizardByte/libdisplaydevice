/**
 * @file src/macos/include/display_device/macos/mac_display_device_interface.h
 * @brief Declarations for the MacDisplayDeviceInterface.
 */
#pragma once

// local includes
#include "types.h"

namespace display_device {
  /**
   * @brief Higher level abstracted API for interacting with macOS display devices.
   */
  class MacDisplayDeviceInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~MacDisplayDeviceInterface() = default;

    /**
     * @brief Check if the API for changing display settings is accessible.
     * @returns True if display settings can be changed, false otherwise.
     */
    [[nodiscard]] virtual bool isApiAccessAvailable() const = 0;

    /**
     * @brief Enumerate the available display devices.
     * @returns A list of available devices. Empty list can also indicate an error.
     */
    [[nodiscard]] virtual EnumeratedDeviceList enumAvailableDevices() const = 0;

    /**
     * @brief Get display name associated with the device.
     * @param device_id A device to get display name for.
     * @returns A display name for the device, or an empty string if not found.
     */
    [[nodiscard]] virtual std::string getDisplayName(const std::string &device_id) const = 0;

    /**
     * @brief Get the active topology.
     * @returns Active topology, or an empty topology if unavailable.
     */
    [[nodiscard]] virtual MacActiveTopology getCurrentTopology() const = 0;

    /**
     * @brief Verify if the active topology is valid.
     * @param topology Topology to validate.
     * @returns True if valid, false otherwise.
     */
    [[nodiscard]] virtual bool isTopologyValid(const MacActiveTopology &topology) const = 0;

    /**
     * @brief Check if the topologies are close enough to be considered the same by macOS.
     * @param lhs First topology to compare.
     * @param rhs Second topology to compare.
     * @returns True if topologies are the same, false otherwise.
     */
    [[nodiscard]] virtual bool isTopologyTheSame(const MacActiveTopology &lhs, const MacActiveTopology &rhs) const = 0;

    /**
     * @brief Set a new active topology.
     * @param new_topology New topology to set.
     * @returns True if the new topology has been set, false otherwise.
     */
    [[nodiscard]] virtual bool setTopology(const MacActiveTopology &new_topology) = 0;

    /**
     * @brief Get current display modes for the devices.
     * @param device_ids Devices to get modes for.
     * @returns Display mode map, or an empty map if unavailable.
     */
    [[nodiscard]] virtual MacDeviceDisplayModeMap getCurrentDisplayModes(const StringSet &device_ids) const = 0;

    /**
     * @brief Set new display modes for the devices.
     * @param modes Modes to set.
     * @returns True if modes were set, false otherwise.
     */
    [[nodiscard]] virtual bool setDisplayModes(const MacDeviceDisplayModeMap &modes) = 0;

    /**
     * @brief Check whether the specified device is primary.
     * @param device_id Device to perform the check for.
     * @returns True if the device is primary, false otherwise.
     */
    [[nodiscard]] virtual bool isPrimary(const std::string &device_id) const = 0;

    /**
     * @brief Set the device as a primary display.
     * @param device_id Device to set as primary.
     * @returns True if the device is or was set as primary, false otherwise.
     */
    [[nodiscard]] virtual bool setAsPrimary(const std::string &device_id) = 0;

    /**
     * @brief Get HDR state for the devices.
     * @param device_ids Devices to get HDR states for.
     * @returns HDR states per device, or an empty map if unavailable.
     */
    [[nodiscard]] virtual MacHdrStateMap getCurrentHdrStates(const StringSet &device_ids) const = 0;

    /**
     * @brief Set HDR states for the devices.
     * @param states HDR states to set.
     * @returns True if HDR states were set or no changes were needed, false otherwise.
     */
    [[nodiscard]] virtual bool setHdrStates(const MacHdrStateMap &states) = 0;
  };
}  // namespace display_device
