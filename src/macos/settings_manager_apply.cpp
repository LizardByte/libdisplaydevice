/**
 * @file src/macos/settings_manager_apply.cpp
 * @brief Definitions for the methods for applying settings in MacSettingsManager.
 */
// class header include
#include "display_device/macos/settings_manager.h"

// system includes
#include <algorithm>
#include <iterator>
#include <optional>
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

    /**
     * @brief Data prepared before applying macOS display settings.
     */
    struct MacApplyPlan {
      MacSingleDisplayConfigState m_state;  ///< New persistence state.
      std::string m_device_to_configure;  ///< Device selected for configuration.
      StringSet m_additional_devices_to_configure;  ///< Additional devices affected by primary-device configuration.
      MacDeviceDisplayModeMap m_cached_display_modes;  ///< Original display modes from cached state.
      bool m_configuring_primary_devices {};  ///< True when no explicit device was requested.
    };

    /**
     * @brief Rollback data for display mode changes.
     */
    struct ModeRollbackState {
      bool m_required {};  ///< True when mode rollback should run on later failure.
      MacDeviceDisplayModeMap m_modes;  ///< Modes to restore.
    };

    /**
     * @brief Build a set from mode map keys.
     * @param modes Mode map to inspect.
     * @return Set containing the mode map keys.
     */
    [[nodiscard]] StringSet getModeKeys(const MacDeviceDisplayModeMap &modes) {
      const auto mode_keys_view {std::views::keys(modes)};
      return {std::begin(mode_keys_view), std::end(mode_keys_view)};
    }

    /**
     * @brief Prepare state and target metadata before applying settings.
     * @param dd_api macOS display-device API.
     * @param config Requested single-display configuration.
     * @param cached_state Previously persisted state, if any.
     * @return Prepared apply data, or empty optional on failure.
     */
    [[nodiscard]] std::optional<MacApplyPlan> createApplyPlan(
      const MacDisplayDeviceInterface &dd_api,
      const SingleDisplayConfiguration &config,
      const std::optional<MacSingleDisplayConfigState> &cached_state
    ) {
      const auto topology_before_changes {dd_api.getCurrentTopology()};
      if (!dd_api.isTopologyValid(topology_before_changes)) {
        DD_LOG(error) << "Retrieved current macOS topology is invalid:\n"
                      << toJson(topology_before_changes);
        return std::nullopt;
      }

      const auto devices {dd_api.enumAvailableDevices()};
      if (devices.empty()) {
        DD_LOG(error) << "Failed to enumerate macOS display devices!";
        return std::nullopt;
      }

      const auto new_initial_state {mac_utils::computeInitialState(cached_state ? std::make_optional(cached_state->m_initial) : std::nullopt, topology_before_changes, devices)};
      if (!new_initial_state) {
        return std::nullopt;
      }

      auto new_state {MacSingleDisplayConfigState {*new_initial_state}};
      const auto stripped_initial_state {mac_utils::stripInitialState(new_state.m_initial, devices)};
      if (!stripped_initial_state) {
        return std::nullopt;
      }

      const bool configuring_primary_devices {config.m_device_id.empty()};
      const auto device_to_configure {configuring_primary_devices ? *std::begin(stripped_initial_state->m_primary_devices) : config.m_device_id};
      const auto additional_devices_to_configure {
        configuring_primary_devices ? makeAdditionalPrimaryDevices(stripped_initial_state->m_primary_devices) : getOtherDevicesInTheSameGroup(topology_before_changes, device_to_configure)
      };

      if (!isActiveDevice(devices, device_to_configure) || !mac_utils::flattenTopology(topology_before_changes).contains(device_to_configure)) {
        DD_LOG(error) << "macOS device " << toJson(device_to_configure, JSON_COMPACT) << " is not active!";
        return std::nullopt;
      }

      new_state.m_modified.m_topology = topology_before_changes;
      return MacApplyPlan {
        new_state,
        device_to_configure,
        additional_devices_to_configure,
        cached_state ? cached_state->m_modified.m_original_modes : MacDeviceDisplayModeMap {},
        configuring_primary_devices
      };
    }

    /**
     * @brief Apply and verify display modes.
     * @param dd_api macOS display-device API.
     * @param current_modes Current display modes before the change.
     * @param new_modes New display modes to apply.
     * @param rollback_state Rollback state to update if modes changed.
     * @return True if the change succeeds or no change is required.
     */
    [[nodiscard]] bool changeDisplayModes(
      MacDisplayDeviceInterface &dd_api,
      const MacDeviceDisplayModeMap &current_modes,
      const MacDeviceDisplayModeMap &new_modes,
      ModeRollbackState &rollback_state
    ) {
      if (current_modes == new_modes) {
        return true;
      }

      DD_LOG(info) << "Changing macOS display modes to:\n"
                   << toJson(new_modes);
      if (!dd_api.setDisplayModes(new_modes)) {
        return false;
      }

      const auto verified_modes {dd_api.getCurrentDisplayModes(getModeKeys(new_modes))};
      if (verified_modes.empty()) {
        DD_LOG(error) << "Failed to verify changed macOS display modes!";
        static_cast<void>(dd_api.setDisplayModes(current_modes));
        return false;
      }

      if (current_modes != verified_modes) {
        rollback_state.m_required = true;
        rollback_state.m_modes = current_modes;
      }

      return true;
    }

    /**
     * @brief Apply requested display mode changes.
     * @param dd_api macOS display-device API.
     * @param config Requested single-display configuration.
     * @param current_modes Current display modes before the change.
     * @param plan Prepared apply state to update.
     * @param rollback_state Rollback state to update if modes changed.
     * @return Apply result for the display-mode stage.
     */
    [[nodiscard]] MacSettingsManager::ApplyResult applyRequestedModes(
      MacDisplayDeviceInterface &dd_api,
      const SingleDisplayConfiguration &config,
      const MacDeviceDisplayModeMap &current_modes,
      MacApplyPlan &plan,
      ModeRollbackState &rollback_state
    ) {
      using enum SettingsManagerInterface::ApplyResult;

      const auto original_display_modes {plan.m_cached_display_modes.empty() ? current_modes : plan.m_cached_display_modes};
      if (const auto new_display_modes {mac_utils::computeNewDisplayModes(config.m_resolution, config.m_refresh_rate, plan.m_configuring_primary_devices, plan.m_device_to_configure, plan.m_additional_devices_to_configure, original_display_modes)}; !changeDisplayModes(dd_api, current_modes, new_display_modes, rollback_state)) {
        DD_LOG(error) << "Failed to apply new macOS display modes!";
        return DisplayModePrepFailed;
      }

      plan.m_state.m_modified.m_original_modes = original_display_modes;
      return Ok;
    }

    /**
     * @brief Apply or restore display modes for an apply request.
     * @param dd_api macOS display-device API.
     * @param config Requested single-display configuration.
     * @param plan Prepared apply state to update.
     * @param rollback_state Rollback state to update if modes changed.
     * @return Apply result for the display-mode stage.
     */
    [[nodiscard]] MacSettingsManager::ApplyResult applyDisplayModes(
      MacDisplayDeviceInterface &dd_api,
      const SingleDisplayConfiguration &config,
      MacApplyPlan &plan,
      ModeRollbackState &rollback_state
    ) {
      using enum SettingsManagerInterface::ApplyResult;

      const bool change_required {config.m_resolution || config.m_refresh_rate};
      if (const bool might_need_to_restore {!plan.m_cached_display_modes.empty()}; !change_required && !might_need_to_restore) {
        return Ok;
      }

      const auto topology_devices {mac_utils::flattenTopology(plan.m_state.m_modified.m_topology)};
      const auto current_display_modes {dd_api.getCurrentDisplayModes(topology_devices)};
      if (current_display_modes.empty()) {
        DD_LOG(error) << "Failed to get current macOS display modes!";
        return DisplayModePrepFailed;
      }

      if (change_required) {
        return applyRequestedModes(dd_api, config, current_display_modes, plan, rollback_state);
      }

      if (!changeDisplayModes(dd_api, current_display_modes, plan.m_cached_display_modes, rollback_state)) {
        DD_LOG(error) << "Failed to restore original macOS display modes!";
        return DisplayModePrepFailed;
      }

      return Ok;
    }
  }  // namespace

  MacSettingsManager::ApplyResult MacSettingsManager::applySettings(const SingleDisplayConfiguration &config) {
    using enum SettingsManagerInterface::ApplyResult;

    const auto api_access {m_dd_api->isApiAccessAvailable()};
    DD_LOG(info) << "Trying to apply macOS display device settings. API is available: " << toJson(api_access);

    if (!api_access) {
      return ApiTemporarilyUnavailable;
    }

    DD_LOG(info) << "Using the following macOS configuration:\n"
                 << toJson(config);

    if (config.m_hdr_state) {
      return HdrStatePrepFailed;
    }

    if (config.m_device_prep != SingleDisplayConfiguration::DevicePreparation::VerifyOnly) {
      DD_LOG(error) << "macOS phase 2 only supports VerifyOnly device preparation.";
      return DevicePrepFailed;
    }

    const auto &cached_state {m_persistence_state->getState()};
    auto apply_plan {createApplyPlan(*m_dd_api, config, cached_state)};
    if (!apply_plan) {
      return DevicePrepFailed;
    }

    ModeRollbackState mode_rollback;
    if (const auto mode_result {applyDisplayModes(*m_dd_api, config, *apply_plan, mode_rollback)}; mode_result != Ok) {
      return mode_result;
    }

    if (!m_persistence_state->persistState(apply_plan->m_state)) {
      DD_LOG(error) << "Failed to persist macOS display settings! Undoing changes...";
      if (mode_rollback.m_required) {
        static_cast<void>(m_dd_api->setDisplayModes(mode_rollback.m_modes));
      }
      return PersistenceSaveFailed;
    }

    return Ok;
  }
}  // namespace display_device
