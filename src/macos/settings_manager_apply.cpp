/**
 * @file src/macos/settings_manager_apply.cpp
 * @brief Definitions for the methods for applying settings in MacSettingsManager.
 */
// class header include
#include "display_device/macos/settings_manager.h"

// system includes
#include <algorithm>
#include <iterator>
#include <ranges>

// local includes
#include "display_device/logging.h"
#include "display_device/macos/json.h"
#include "display_device/macos/settings_utils.h"

namespace display_device {
  namespace {
    /**
     * @brief Check whether a device is active in the enumerated device list.
     * @param devices Devices to inspect.
     * @param device_id Device id to find.
     * @return True if the device exists and is active.
     */
    [[nodiscard]] bool isActiveDevice(const EnumeratedDeviceList &devices, const std::string &device_id) {
      const auto device_it {std::ranges::find_if(devices, [&device_id](const auto &device) {
        return device.m_device_id == device_id;
      })};

      return device_it != std::end(devices) && device_it->m_info.has_value();
    }

    /**
     * @brief Find other devices in the same topology group as a target device.
     * @param topology Topology to inspect.
     * @param target_device_id Target device id.
     * @return Devices from the same group except the target.
     */
    [[nodiscard]] StringSet getOtherDevicesInTheSameGroup(const MacActiveTopology &topology, const std::string &target_device_id) {
      StringSet device_ids;
      for (const auto &group : topology) {
        if (std::ranges::find(group, target_device_id) == std::end(group)) {
          continue;
        }

        std::ranges::copy_if(group, std::inserter(device_ids, std::begin(device_ids)), [&target_device_id](const auto &device_id) {
          return device_id != target_device_id;
        });
        break;
      }

      return device_ids;
    }

    /**
     * @brief Create additional devices to configure for primary-device requests.
     * @param primary_devices Primary devices from the initial state.
     * @return Primary devices except the first one.
     */
    [[nodiscard]] StringSet makeAdditionalPrimaryDevices(const StringSet &primary_devices) {
      if (primary_devices.empty()) {
        return {};
      }

      return {std::next(std::begin(primary_devices)), std::end(primary_devices)};
    }
  }  // namespace

  MacSettingsManager::ApplyResult MacSettingsManager::applySettings(const SingleDisplayConfiguration &config) {
    const auto api_access {m_dd_api->isApiAccessAvailable()};
    DD_LOG(info) << "Trying to apply macOS display device settings. API is available: " << toJson(api_access);

    if (!api_access) {
      return ApplyResult::ApiTemporarilyUnavailable;
    }

    DD_LOG(info) << "Using the following macOS configuration:\n"
                 << toJson(config);

    if (config.m_hdr_state) {
      return ApplyResult::HdrStatePrepFailed;
    }

    if (config.m_device_prep != SingleDisplayConfiguration::DevicePreparation::VerifyOnly) {
      DD_LOG(error) << "macOS phase 2 only supports VerifyOnly device preparation.";
      return ApplyResult::DevicePrepFailed;
    }

    const auto topology_before_changes {m_dd_api->getCurrentTopology()};
    if (!m_dd_api->isTopologyValid(topology_before_changes)) {
      DD_LOG(error) << "Retrieved current macOS topology is invalid:\n"
                    << toJson(topology_before_changes);
      return ApplyResult::DevicePrepFailed;
    }

    const auto devices {m_dd_api->enumAvailableDevices()};
    if (devices.empty()) {
      DD_LOG(error) << "Failed to enumerate macOS display devices!";
      return ApplyResult::DevicePrepFailed;
    }

    const auto &cached_state {m_persistence_state->getState()};
    const auto new_initial_state {mac_utils::computeInitialState(cached_state ? std::make_optional(cached_state->m_initial) : std::nullopt, topology_before_changes, devices)};
    if (!new_initial_state) {
      return ApplyResult::DevicePrepFailed;
    }

    auto new_state {MacSingleDisplayConfigState {*new_initial_state}};
    const auto stripped_initial_state {mac_utils::stripInitialState(new_state.m_initial, devices)};
    if (!stripped_initial_state) {
      return ApplyResult::DevicePrepFailed;
    }

    const bool configuring_primary_devices {config.m_device_id.empty()};
    const auto device_to_configure {configuring_primary_devices ? *std::begin(stripped_initial_state->m_primary_devices) : config.m_device_id};
    const auto additional_devices_to_configure {
      configuring_primary_devices ? makeAdditionalPrimaryDevices(stripped_initial_state->m_primary_devices) : getOtherDevicesInTheSameGroup(topology_before_changes, device_to_configure)
    };

    if (!isActiveDevice(devices, device_to_configure) || !mac_utils::flattenTopology(topology_before_changes).contains(device_to_configure)) {
      DD_LOG(error) << "macOS device " << toJson(device_to_configure, JSON_COMPACT) << " is not active!";
      return ApplyResult::DevicePrepFailed;
    }

    new_state.m_modified.m_topology = topology_before_changes;

    const auto cached_display_modes {cached_state ? cached_state->m_modified.m_original_modes : MacDeviceDisplayModeMap {}};
    const bool change_required {config.m_resolution || config.m_refresh_rate};
    const bool might_need_to_restore {!cached_display_modes.empty()};

    bool rollback_modes_on_failure {false};
    MacDeviceDisplayModeMap modes_to_restore;

    if (change_required || might_need_to_restore) {
      const auto topology_devices {mac_utils::flattenTopology(new_state.m_modified.m_topology)};
      const auto current_display_modes {m_dd_api->getCurrentDisplayModes(topology_devices)};
      if (current_display_modes.empty()) {
        DD_LOG(error) << "Failed to get current macOS display modes!";
        return ApplyResult::DisplayModePrepFailed;
      }

      const auto try_change = [this, &rollback_modes_on_failure, &modes_to_restore, &current_display_modes](const MacDeviceDisplayModeMap &new_modes) {
        if (current_display_modes == new_modes) {
          return true;
        }

        DD_LOG(info) << "Changing macOS display modes to:\n"
                     << toJson(new_modes);
        if (!m_dd_api->setDisplayModes(new_modes)) {
          return false;
        }

        const auto mode_keys_view {std::views::keys(new_modes)};
        const StringSet mode_keys {std::begin(mode_keys_view), std::end(mode_keys_view)};
        const auto verified_modes {m_dd_api->getCurrentDisplayModes(mode_keys)};
        if (verified_modes.empty()) {
          DD_LOG(error) << "Failed to verify changed macOS display modes!";
          static_cast<void>(m_dd_api->setDisplayModes(current_display_modes));
          return false;
        }

        if (current_display_modes != verified_modes) {
          rollback_modes_on_failure = true;
          modes_to_restore = current_display_modes;
        }

        return true;
      };

      if (change_required) {
        const auto original_display_modes {cached_display_modes.empty() ? current_display_modes : cached_display_modes};
        const auto new_display_modes {
          mac_utils::computeNewDisplayModes(config.m_resolution, config.m_refresh_rate, configuring_primary_devices, device_to_configure, additional_devices_to_configure, original_display_modes)
        };

        if (!try_change(new_display_modes)) {
          DD_LOG(error) << "Failed to apply new macOS display modes!";
          return ApplyResult::DisplayModePrepFailed;
        }

        new_state.m_modified.m_original_modes = original_display_modes;
      } else if (!try_change(cached_display_modes)) {
        DD_LOG(error) << "Failed to restore original macOS display modes!";
        return ApplyResult::DisplayModePrepFailed;
      }
    }

    if (!m_persistence_state->persistState(new_state)) {
      DD_LOG(error) << "Failed to persist macOS display settings! Undoing changes...";
      if (rollback_modes_on_failure) {
        static_cast<void>(m_dd_api->setDisplayModes(modes_to_restore));
      }
      return ApplyResult::PersistenceSaveFailed;
    }

    return ApplyResult::Ok;
  }
}  // namespace display_device
