#pragma once

// local includes
#include "displaydevice/windows/types.h"

namespace display_device {
  /**
   * @brief Higher level abstracted API for interacting Windows' display device(s).
   */
  class WinDisplayDeviceInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~WinDisplayDeviceInterface() = default;

    /**
     * @brief Get the active (current) topology.
     * @returns A list representing the current topology.
     *          Empty list can also be returned if an error has occurred.
     *
     * EXAMPLES:
     * ```cpp
     * const WinDisplayDeviceInterface* iface = getIface(...);
     * const auto current_topology { iface->getCurrentTopology() };
     * ```
     */
    [[nodiscard]] virtual ActiveTopology
    getCurrentTopology() const = 0;

    /**
     * @brief Verify if the active topology is valid.
     *
     * This is mostly meant as a sanity check or to verify that it is still valid
     * after a manual modification to an existing topology.
     *
     * @param topology Topology to validate.
     * @returns True if it is valid, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * const WinDisplayDeviceInterface* iface = getIface(...);
     * auto current_topology { iface->getCurrentTopology() };
     * // Modify the current_topology
     * const bool is_valid = iface->isTopologyValid(current_topology);
     * ```
     */
    [[nodiscard]] virtual bool
    isTopologyValid(const ActiveTopology &topology) const = 0;

    /**
     * @brief Check if the topologies are close enough to be considered the same by the OS.
     * @param lhs First topology to compare.
     * @param rhs Second topology to compare.
     * @returns True if topologies are close enough, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * const WinDisplayDeviceInterface* iface = getIface(...);
     * auto current_topology { iface->getCurrentTopology() };
     * auto new_topology { current_topology };
     * // Modify the new_topology
     * const bool is_the_same = iface->isTopologyTheSame(current_topology, new_topology);
     * ```
     */
    [[nodiscard]] virtual bool
    isTopologyTheSame(const ActiveTopology &lhs, const ActiveTopology &rhs) const = 0;

    /**
     * @brief Set a new active topology for the OS.
     * @param new_topology New device topology to set.
     * @returns True if the new topology has been set, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * auto current_topology { getCurrentTopology() };
     * // Modify the current_topology
     * const bool success = setTopology(current_topology);
     * ```
     */
    [[nodiscard]] virtual bool
    setTopology(const ActiveTopology &new_topology) = 0;
  };
}  // namespace display_device
