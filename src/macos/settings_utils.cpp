/**
 * @file src/macos/settings_utils.cpp
 * @brief Definitions for macOS settings utility functions.
 */
// header include
#include "display_device/macos/settings_utils.h"

// system includes
#include <algorithm>
#include <cmath>
#include <iterator>
#include <type_traits>
#include <variant>
#include <vector>

// local includes
#include "display_device/logging.h"
#include "display_device/macos/json.h"

namespace display_device::mac_utils {
  namespace {
    /**
     * @brief Predicate that accepts any device.
     * @param device Device to check.
     * @return Always true.
     */
    [[nodiscard]] bool anyDevice(const EnumeratedDevice &device) {
      static_cast<void>(device);
      return true;
    }

    /**
     * @brief Predicate that accepts primary active devices.
     * @param device Device to check.
     * @return True if the device is active and primary.
     */
    [[nodiscard]] bool primaryOnlyDevices(const EnumeratedDevice &device) {
      return device.m_info && device.m_info->m_primary;
    }

    /**
     * @brief Get device ids from devices matching a predicate.
     * @param devices Device list to inspect.
     * @param predicate Predicate to match.
     * @return Matching device ids.
     */
    [[nodiscard]] StringSet getDeviceIds(const EnumeratedDeviceList &devices, std::add_lvalue_reference_t<bool(const EnumeratedDevice &)> &predicate) {
      StringSet device_ids;
      for (const auto &device : devices) {
        if (predicate(device)) {
          device_ids.insert(device.m_device_id);
        }
      }

      return device_ids;
    }

    /**
     * @brief Remove unavailable devices from a topology.
     * @param topology Topology to strip.
     * @param devices Currently available devices.
     * @return Topology containing only available devices.
     */
    [[nodiscard]] MacActiveTopology stripTopology(const MacActiveTopology &topology, const EnumeratedDeviceList &devices) {
      const StringSet available_device_ids {getDeviceIds(devices, anyDevice)};

      MacActiveTopology stripped_topology;
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
     * @brief Remove unavailable device ids.
     * @param device_ids Device ids to strip.
     * @param devices Currently available devices.
     * @return Device ids containing only available devices.
     */
    [[nodiscard]] StringSet stripDevices(const StringSet &device_ids, const EnumeratedDeviceList &devices) {
      const StringSet available_device_ids {getDeviceIds(devices, anyDevice)};

      StringSet available_devices;
      std::ranges::set_intersection(device_ids, available_device_ids, std::inserter(available_devices, std::begin(available_devices)));
      return available_devices;
    }

    /**
     * @brief Merge the primary and additional devices into one ordered list.
     * @param device_to_configure Primary device to configure.
     * @param additional_devices_to_configure Additional devices to configure.
     * @return Ordered device id list.
     */
    [[nodiscard]] std::vector<std::string> joinConfigurableDevices(const std::string &device_to_configure, const StringSet &additional_devices_to_configure) {
      std::vector<std::string> devices {device_to_configure};
      devices.insert(std::end(devices), std::begin(additional_devices_to_configure), std::end(additional_devices_to_configure));
      return devices;
    }

    /**
     * @brief Convert a floating-point setting to a rational refresh rate.
     * @param value Floating-point setting.
     * @return Rational refresh rate.
     */
    [[nodiscard]] Rational fromFloatingPoint(const FloatingPoint &value) {
      if (const auto *rational_value {std::get_if<Rational>(&value)}; rational_value) {
        return *rational_value;
      }

      constexpr unsigned int multiplier {10000};
      const auto transformed_value {std::round(std::get<double>(value) * multiplier)};
      return Rational {static_cast<unsigned int>(transformed_value), multiplier};
    }
  }  // namespace

  StringSet flattenTopology(const MacActiveTopology &topology) {
    StringSet flattened_topology;
    for (const auto &group : topology) {
      for (const auto &device_id : group) {
        flattened_topology.insert(device_id);
      }
    }

    return flattened_topology;
  }

  std::string getPrimaryDevice(const MacDisplayDeviceInterface &mac_dd, const MacActiveTopology &topology) {
    for (const auto &device_id : flattenTopology(topology)) {
      if (mac_dd.isPrimary(device_id)) {
        return device_id;
      }
    }

    return {};
  }

  std::optional<MacSingleDisplayConfigState::Initial> computeInitialState(
    const std::optional<MacSingleDisplayConfigState::Initial> &prev_state,
    const MacActiveTopology &topology_before_changes,
    const EnumeratedDeviceList &devices
  ) {
    if (prev_state) {
      return *prev_state;
    }

    const auto primary_devices {getDeviceIds(devices, primaryOnlyDevices)};
    if (primary_devices.empty()) {
      DD_LOG(error) << "Enumerated macOS device list does not contain primary devices!";
      return std::nullopt;
    }

    return MacSingleDisplayConfigState::Initial {
      topology_before_changes,
      primary_devices
    };
  }

  std::optional<MacSingleDisplayConfigState::Initial> stripInitialState(
    const MacSingleDisplayConfigState::Initial &initial_state,
    const EnumeratedDeviceList &devices
  ) {
    const auto stripped_initial_topology {stripTopology(initial_state.m_topology, devices)};
    auto initial_primary_devices {stripDevices(initial_state.m_primary_devices, devices)};

    if (stripped_initial_topology.empty()) {
      DD_LOG(error) << "Enumerated macOS device list does not contain any device from the initial state!";
      return std::nullopt;
    }

    if (initial_primary_devices.empty()) {
      initial_primary_devices = getDeviceIds(devices, primaryOnlyDevices);
      if (initial_primary_devices.empty()) {
        DD_LOG(error) << "Enumerated macOS device list does not contain primary devices!";
        return std::nullopt;
      }
    }

    if (initial_state.m_topology != stripped_initial_topology || initial_state.m_primary_devices != initial_primary_devices) {
      DD_LOG(warning) << "Adapting macOS initial state because some devices are unavailable.\n"
                      << "  - topology: " << toJson(initial_state.m_topology, JSON_COMPACT) << " -> " << toJson(stripped_initial_topology, JSON_COMPACT) << "\n"
                      << "  - primary devices: " << toJson(initial_state.m_primary_devices, JSON_COMPACT) << " -> " << toJson(initial_primary_devices, JSON_COMPACT);
    }

    return MacSingleDisplayConfigState::Initial {
      stripped_initial_topology,
      initial_primary_devices
    };
  }

  MacDeviceDisplayModeMap computeNewDisplayModes(
    const std::optional<Resolution> &resolution,
    const std::optional<FloatingPoint> &refresh_rate,
    const bool configuring_primary_devices,
    const std::string &device_to_configure,
    const StringSet &additional_devices_to_configure,
    const MacDeviceDisplayModeMap &original_modes
  ) {
    MacDeviceDisplayModeMap new_modes {original_modes};

    if (resolution) {
      const auto devices {joinConfigurableDevices(device_to_configure, additional_devices_to_configure)};
      for (const auto &device_id : devices) {
        new_modes[device_id].m_resolution = *resolution;
      }
    }

    if (refresh_rate) {
      if (configuring_primary_devices) {
        const auto devices {joinConfigurableDevices(device_to_configure, additional_devices_to_configure)};
        for (const auto &device_id : devices) {
          new_modes[device_id].m_refresh_rate = fromFloatingPoint(*refresh_rate);
        }
      } else {
        new_modes[device_to_configure].m_refresh_rate = fromFloatingPoint(*refresh_rate);
      }
    }

    return new_modes;
  }

  MacDdGuardFn modeGuardFn(MacDisplayDeviceInterface &mac_dd, const MacDeviceDisplayModeMap &modes) {
    DD_LOG(debug) << "Got macOS modes in modeGuardFn:\n"
                  << toJson(modes);
    return [&mac_dd, modes]() {
      if (!mac_dd.setDisplayModes(modes)) {
        DD_LOG(error) << "Failed to revert macOS display modes in modeGuardFn! Used the following modes:\n"
                      << toJson(modes);
      }
    };
  }

  void noopGuard() {
    // Intentionally empty guard callback.
  }
}  // namespace display_device::mac_utils
