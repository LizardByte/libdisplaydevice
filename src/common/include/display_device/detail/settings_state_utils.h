/**
 * @file src/common/include/display_device/detail/settings_state_utils.h
 * @brief Shared helpers for adapting settings state.
 */
#pragma once

// system includes
#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// local includes
#include "display_device/json.h"
#include "display_device/logging.h"
#include "display_device/types.h"

namespace display_device::detail {
  /**
   * @brief Log messages used while stripping unavailable devices from initial state.
   */
  struct InitialStateStripMessages {
    std::string_view m_missing_topology;  ///< Error logged when no initial topology devices remain.
    std::string_view m_missing_primary;  ///< Error logged when no usable primary devices remain.
    std::string_view m_adapted_state;  ///< Warning prefix logged when the initial state is adapted.
  };

  /**
   * @brief Strip unavailable device ids from a topology.
   * @tparam Topology Topology container type.
   * @param topology Topology to strip.
   * @param available_device_ids Device ids currently available.
   * @return Topology containing only available device ids.
   */
  template<typename Topology>
  [[nodiscard]] Topology stripUnavailableTopology(const Topology &topology, const StringSet &available_device_ids) {
    Topology stripped_topology;
    for (const auto &group : topology) {
      std::vector<std::string> stripped_group;
      for (const auto &device_id : group) {
        if (available_device_ids.contains(device_id)) {
          stripped_group.push_back(device_id);
        }
      }

      if (!stripped_group.empty()) {
        stripped_topology.push_back(stripped_group);
      }
    }

    return stripped_topology;
  }

  /**
   * @brief Strip unavailable devices from an initial settings state.
   * @tparam Initial Initial state type.
   * @tparam FormatTopologyFn Callable type used to format topology values for logs.
   * @param initial_state Initial state to strip.
   * @param available_device_ids Device ids currently available.
   * @param primary_device_ids Current primary device ids.
   * @param messages Log messages to use for failure and adaptation cases.
   * @param format_topology Callable used to format topology values.
   * @return Stripped initial state, or empty optional if no usable state remains.
   */
  template<typename Initial, typename FormatTopologyFn>
  [[nodiscard]] std::optional<Initial> stripInitialState(
    const Initial &initial_state,
    const StringSet &available_device_ids,
    const StringSet &primary_device_ids,
    const InitialStateStripMessages &messages,
    const FormatTopologyFn &format_topology
  ) {
    const auto stripped_initial_topology {stripUnavailableTopology(initial_state.m_topology, available_device_ids)};

    StringSet initial_primary_devices;
    std::ranges::set_intersection(
      initial_state.m_primary_devices,
      available_device_ids,
      std::inserter(initial_primary_devices, std::begin(initial_primary_devices))
    );

    if (stripped_initial_topology.empty()) {
      DD_LOG(error) << messages.m_missing_topology;
      return std::nullopt;
    }

    if (initial_primary_devices.empty()) {
      initial_primary_devices = primary_device_ids;
      if (initial_primary_devices.empty()) {
        DD_LOG(error) << messages.m_missing_primary;
        return std::nullopt;
      }
    }

    if (initial_state.m_topology != stripped_initial_topology || initial_state.m_primary_devices != initial_primary_devices) {
      DD_LOG(warning) << messages.m_adapted_state << "\n"
                      << "  - topology: " << format_topology(initial_state.m_topology) << " -> " << format_topology(stripped_initial_topology) << "\n"
                      << "  - primary devices: " << toJson(initial_state.m_primary_devices, JSON_COMPACT) << " -> " << toJson(initial_primary_devices, JSON_COMPACT);
    }

    return Initial {
      stripped_initial_topology,
      initial_primary_devices
    };
  }
}  // namespace display_device::detail
