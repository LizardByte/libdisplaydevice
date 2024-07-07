// header include
#include "displaydevice/windows/settingsutils.h"

// system includes
#include <algorithm>

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/windows/json.h"

namespace display_device::win_utils {
  namespace {
    /**
     * @brief predicate for getDeviceIds.
     */
    bool
    anyDevice(const EnumeratedDevice &) {
      return true;
    }

    /**
     * @brief predicate for getDeviceIds.
     */
    bool
    primaryOnlyDevices(const EnumeratedDevice &device) {
      return device.m_info && device.m_info->m_primary;
    }

    /**
     * @brief Get device ids from device list matching the predicate.
     * @param devices List of devices.
     * @param predicate Predicate to use.
     * @return Device ids from device list matching the predicate.
     *
     * EXAMPLES:
     * ```cpp
     * const EnumeratedDeviceList devices { ... };
     *
     * const auto all_ids { getDeviceIds(devices, anyDevice) };
     * const auto primary_only_ids { getDeviceIds(devices, primaryOnlyDevices) };
     * ```
     */
    std::set<std::string>
    getDeviceIds(const EnumeratedDeviceList &devices, const std::add_lvalue_reference_t<bool(const EnumeratedDevice &)> &predicate) {
      std::set<std::string> device_ids;
      for (const auto &device : devices) {
        if (predicate(device)) {
          device_ids.insert(device.m_device_id);
        }
      }

      return device_ids;
    }

    /**
     * @brief Remove the topology device ids and groups that no longer have valid devices.
     * @param topology Topology to be stripped.
     * @param devices List of devices.
     * @return Topology without missing device ids.
     */
    ActiveTopology
    stripTopology(const ActiveTopology &topology, const EnumeratedDeviceList &devices) {
      const std::set<std::string> available_device_ids { getDeviceIds(devices, anyDevice) };

      ActiveTopology stripped_topology;
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
     * @brief Remove device ids that are no longer available.
     * @param device_ids Id list to be stripped.
     * @param devices List of devices.
     * @return List without missing device ids.
     */
    std::set<std::string>
    stripDevices(const std::set<std::string> &device_ids, const EnumeratedDeviceList &devices) {
      std::set<std::string> available_device_ids { getDeviceIds(devices, anyDevice) };

      std::set<std::string> available_devices;
      std::ranges::set_intersection(device_ids, available_device_ids,
        std::inserter(available_devices, std::begin(available_devices)));
      return available_devices;
    }

    /**
     * @brief Find topology group with matching id and get other ids from the group.
     * @param topology Topology to be searched.
     * @param target_device_id Device id whose group to search for.
     * @return Other ids in the group without (excluding the provided one).
     */
    std::set<std::string>
    tryGetOtherDevicesInTheSameGroup(const ActiveTopology &topology, const std::string &target_device_id) {
      std::set<std::string> device_ids;

      for (const auto &group : topology) {
        for (const auto &group_device_id : group) {
          if (group_device_id == target_device_id) {
            std::ranges::copy_if(group, std::inserter(device_ids, std::begin(device_ids)), [&target_device_id](const auto &id) {
              return id != target_device_id;
            });
            break;
          }
        }
      }

      return device_ids;
    }

    /**
     * @brief Merge the configurable devices into a vector.
     */
    std::vector<std::string>
    joinConfigurableDevices(const std::string &device_to_configure, const std::set<std::string> &additional_devices_to_configure) {
      std::vector<std::string> devices { device_to_configure };
      devices.insert(std::end(devices), std::begin(additional_devices_to_configure), std::end(additional_devices_to_configure));
      return devices;
    }
  }  // namespace

  std::set<std::string>
  flattenTopology(const ActiveTopology &topology) {
    std::set<std::string> flattened_topology;
    for (const auto &group : topology) {
      for (const auto &device_id : group) {
        flattened_topology.insert(device_id);
      }
    }

    return flattened_topology;
  }

  std::optional<SingleDisplayConfigState::Initial>
  computeInitialState(const std::optional<SingleDisplayConfigState::Initial> &prev_state, const ActiveTopology &topology_before_changes, const EnumeratedDeviceList &devices) {
    // We first need to determine the "initial" state that will be used when reverting
    // the changes as the "go-to" final state we need to achieve. It will also be used
    // as the base for creating new topology based on the provided config settings.
    //
    // Having a constant base allows us to re-apply settings with different configuration
    // parameters without actually reverting the topology back to the initial one where
    // the primary display could have changed in between the first call to this method
    // and a next one.
    //
    // If the user wants to use a "fresh" and "current" system settings, they have to revert
    // changes as otherwise we are using the cached state as a base.
    if (prev_state) {
      return *prev_state;
    }

    const auto primary_devices { getDeviceIds(devices, primaryOnlyDevices) };
    if (primary_devices.empty()) {
      DD_LOG(error) << "Enumerated device list does not contain primary devices!";
      return std::nullopt;
    }

    return SingleDisplayConfigState::Initial {
      topology_before_changes,
      primary_devices
    };
  }

  ActiveTopology
  computeNewTopology(const SingleDisplayConfiguration::DevicePreparation device_prep, const bool configuring_primary_devices, const std::string &device_to_configure, const std::set<std::string> &additional_devices_to_configure, const ActiveTopology &initial_topology) {
    using DevicePrep = SingleDisplayConfiguration::DevicePreparation;

    if (device_prep != DevicePrep::VerifyOnly) {
      if (device_prep == DevicePrep::EnsureOnlyDisplay) {
        // Device needs to be the only one that's active OR if it's a PRIMARY device,
        // only the whole PRIMARY group needs to be active (in case they are duplicated)
        if (configuring_primary_devices) {
          return ActiveTopology { joinConfigurableDevices(device_to_configure, additional_devices_to_configure) };
        }

        return ActiveTopology { { device_to_configure } };
      }
      // DevicePrep::EnsureActive || DevicePrep::EnsurePrimary
      else {
        //  The device needs to be active at least.
        if (!flattenTopology(initial_topology).contains(device_to_configure)) {
          // Create an extended topology as it's probably what makes sense the most...
          ActiveTopology new_topology { initial_topology };
          new_topology.push_back({ device_to_configure });
          return new_topology;
        }
      }
    }

    return initial_topology;
  }

  std::optional<SingleDisplayConfigState::Initial>
  stripInitialState(const SingleDisplayConfigState::Initial &initial_state, const EnumeratedDeviceList &devices) {
    const auto stripped_initial_topology { stripTopology(initial_state.m_topology, devices) };
    auto initial_primary_devices { stripDevices(initial_state.m_primary_devices, devices) };

    if (stripped_initial_topology.empty()) {
      DD_LOG(error) << "Enumerated device list does not contain ANY of the devices from the initial state!";
      return std::nullopt;
    }

    if (initial_primary_devices.empty()) {
      // The initial primay device is no longer available, so maybe it makes sense to use the current one. Maybe...
      initial_primary_devices = getDeviceIds(devices, primaryOnlyDevices);
      if (initial_primary_devices.empty()) {
        DD_LOG(error) << "Enumerated device list does not contain primary devices!";
        return std::nullopt;
      }
    }

    if (initial_state.m_topology != stripped_initial_topology || initial_state.m_primary_devices != initial_primary_devices) {
      DD_LOG(warning) << "Trying to apply configuration without reverting back to initial topology first, however not all devices from that "
                         "topology are available.\n"
                      << "Will try adapting the initial topology that is used as a base:\n"
                      << "  - topology: " << toJson(initial_state.m_topology, JSON_COMPACT) << " -> " << toJson(stripped_initial_topology, JSON_COMPACT) << "\n"
                      << "  - primary devices: " << toJson(initial_state.m_primary_devices, JSON_COMPACT) << " -> " << toJson(initial_primary_devices, JSON_COMPACT);
    }

    return SingleDisplayConfigState::Initial {
      stripped_initial_topology,
      initial_primary_devices
    };
  }

  std::tuple<ActiveTopology, std::string, std::set<std::string>>
  computeNewTopologyAndMetadata(const SingleDisplayConfiguration::DevicePreparation device_prep, const std::string &device_id, const SingleDisplayConfigState::Initial &initial_state) {
    const bool configuring_unspecified_devices { device_id.empty() };
    const auto device_to_configure { configuring_unspecified_devices ? *std::begin(initial_state.m_primary_devices) : device_id };
    auto additional_devices_to_configure { configuring_unspecified_devices ?
                                             std::set<std::string> { std::next(std::begin(initial_state.m_primary_devices)), std::end(initial_state.m_primary_devices) } :
                                             tryGetOtherDevicesInTheSameGroup(initial_state.m_topology, device_to_configure) };
    DD_LOG(info) << "Will compute new display device topology from the following input:\n"
                 << "  - initial topology: " << toJson(initial_state.m_topology, JSON_COMPACT) << "\n"
                 << "  - initial primary devices: " << toJson(initial_state.m_primary_devices, JSON_COMPACT) << "\n"
                 << "  - configuring unspecified device: " << toJson(configuring_unspecified_devices, JSON_COMPACT) << "\n"
                 << "  - device to configure: " << toJson(device_to_configure, JSON_COMPACT) << "\n"
                 << "  - additional devices to configure: " << toJson(additional_devices_to_configure, JSON_COMPACT);

    const auto new_topology { computeNewTopology(device_prep, configuring_unspecified_devices, device_to_configure, additional_devices_to_configure, initial_state.m_topology) };
    additional_devices_to_configure = tryGetOtherDevicesInTheSameGroup(new_topology, device_to_configure);
    return std::make_tuple(new_topology, device_to_configure, additional_devices_to_configure);
  }

  DdGuardFn
  topologyGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology) {
    DD_LOG(debug) << "Got topology in topologyGuardFn:\n"
                  << toJson(topology);
    return [&win_dd, topology]() {
      if (!win_dd.setTopology(topology)) {
        DD_LOG(error) << "failed to revert topology in topologyGuardFn! Used the following topology:\n"
                      << toJson(topology);
      }
    };
  }

  DdGuardFn
  modeGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology) {
    const auto modes = win_dd.getCurrentDisplayModes(flattenTopology(topology));
    DD_LOG(debug) << "Got modes in modeGuardFn:\n"
                  << toJson(modes);
    return [&win_dd, modes]() {
      if (!win_dd.setDisplayModes(modes)) {
        DD_LOG(error) << "failed to revert display modes in modeGuardFn! Used the following modes:\n"
                      << toJson(modes);
      }
    };
  }

  DdGuardFn
  primaryGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology) {
    std::string primary_device {};
    const auto flat_topology { flattenTopology(topology) };
    for (const auto &device_id : flat_topology) {
      if (win_dd.isPrimary(device_id)) {
        primary_device = device_id;
        break;
      }
    }

    DD_LOG(debug) << "Got primary device in primaryGuardFn:\n"
                  << toJson(primary_device);
    return [&win_dd, primary_device]() {
      if (!win_dd.setAsPrimary(primary_device)) {
        DD_LOG(error) << "failed to revert primary device in primaryGuardFn! Used the following device id:\n"
                      << toJson(primary_device);
      }
    };
  }

  DdGuardFn
  hdrStateGuardFn(WinDisplayDeviceInterface &win_dd, const ActiveTopology &topology) {
    const auto states = win_dd.getCurrentHdrStates(flattenTopology(topology));
    DD_LOG(debug) << "Got states in hdrStateGuardFn:\n"
                  << toJson(states);
    return [&win_dd, states]() {
      if (!win_dd.setHdrStates(states)) {
        DD_LOG(error) << "failed to revert HDR states in hdrStateGuardFn! Used the following HDR states:\n"
                      << toJson(states);
      }
    };
  }
}  // namespace display_device::win_utils
