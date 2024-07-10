// class header include
#include "displaydevice/windows/settingsmanager.h"

// system includes
#include <algorithm>
#include <boost/scope/scope_exit.hpp>

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/windows/json.h"
#include "displaydevice/windows/settingsutils.h"

namespace display_device {
  namespace {
    /**
     * @brief Function that does nothing.
     */
    void
    noopFn() {
    }
  }  // namespace

  SettingsManager::ApplyResult
  SettingsManager::applySettings(const SingleDisplayConfiguration &config) {
    const auto api_access { m_dd_api->isApiAccessAvailable() };
    DD_LOG(info) << "Trying to apply display device settings. API is available: " << toJson(api_access);

    if (!api_access) {
      return ApplyResult::ApiTemporarilyUnavailable;
    }
    DD_LOG(info) << "Using the following configuration:\n"
                 << toJson(config);

    const auto topology_before_changes { m_dd_api->getCurrentTopology() };
    if (!m_dd_api->isTopologyValid(topology_before_changes)) {
      DD_LOG(error) << "Retrieved current topology is invalid:\n"
                    << toJson(topology_before_changes);
      return ApplyResult::DevicePrepFailed;
    }
    DD_LOG(info) << "Active topology before any changes:\n"
                 << toJson(topology_before_changes);

    bool release_context { false };
    boost::scope::scope_exit topology_prep_guard { [this, topology = topology_before_changes, was_captured = m_audio_context_api->isCaptured(), &release_context]() {
      // It is possible that during topology preparation, some settings will be reverted for the modified topology.
      // To keel it simple, these settings will not be restored!
      const auto result { m_dd_api->setTopology(topology) };
      if (!result) {
        DD_LOG(error) << "Failed to revert back to topology in the topology guard!";
        if (release_context) {
          // We are currently in the topology for which the context was captured.
          // We have also failed to revert back to some previous one, so we remain in this topology for
          // which we have context. There is no reason to keep it around then...
          m_audio_context_api->release();
        }
      }

      if (!was_captured && m_audio_context_api->isCaptured()) {
        // We only want to release context that was not captured before.
        m_audio_context_api->release();
      }
    } };

    const auto &prepped_topology_data { prepareTopology(config, topology_before_changes, release_context) };
    if (!prepped_topology_data) {
      // Error already logged
      return ApplyResult::DevicePrepFailed;
    }
    auto [new_state, device_to_configure, additional_devices_to_configure] = *prepped_topology_data;

    DdGuardFn primary_guard_fn { noopFn };
    boost::scope::scope_exit<DdGuardFn &> primary_guard { primary_guard_fn };
    if (!preparePrimaryDevice(config, device_to_configure, primary_guard_fn, new_state)) {
      // Error already logged
      return ApplyResult::PrimaryDevicePrepFailed;
    }

    // TODO:
    //
    //    Other device handling goes here that will use device_to_configure and additional_devices_to_configure:
    //
    //      - handle display modes
    //      - handle HDR (need to replicate the HDR bug and find the best place for workaround)
    //

    // We will always keep the new state persistently, even if there are no new meaningful changes, because
    // we want to preserve the initial state for consistency.
    if (!m_persistence_state->persistState(new_state)) {
      DD_LOG(error) << "Failed to save reverted settings! Undoing everything...";
      return ApplyResult::PersistenceSaveFailed;
    }

    // We can only release the context now as nothing else can fail.
    if (release_context) {
      m_audio_context_api->release();
    }

    // Disable all guards before returning
    topology_prep_guard.set_active(false);
    primary_guard.set_active(false);
    return ApplyResult::Ok;
  }

  std::optional<std::tuple<SingleDisplayConfigState, std::string, std::set<std::string>>>
  SettingsManager::prepareTopology(const SingleDisplayConfiguration &config, const ActiveTopology &topology_before_changes, bool &release_context) {
    const EnumeratedDeviceList devices { m_dd_api->enumAvailableDevices() };
    if (devices.empty()) {
      DD_LOG(error) << "Failed to enumerate display devices!";
      return std::nullopt;
    }
    DD_LOG(info) << "Currently available devices:\n"
                 << toJson(devices);

    if (!config.m_device_id.empty()) {
      auto device_it { std::ranges::find_if(devices, [device_id = config.m_device_id](const auto &item) { return item.m_device_id == device_id; }) };
      if (device_it == std::end(devices)) {
        // Do not use toJson in case the user entered some BS string...
        DD_LOG(error) << "Device \"" << config.m_device_id << "\" is not available in the system!";
        return std::nullopt;
      }
    }

    const auto &cached_state { m_persistence_state->getState() };
    const auto new_initial_state { win_utils::computeInitialState(cached_state ? std::make_optional(cached_state->m_initial) : std::nullopt, topology_before_changes, devices) };
    if (!new_initial_state) {
      // Error already logged
      return std::nullopt;
    }
    SingleDisplayConfigState new_state { *new_initial_state };

    // In case some devices are no longer available in the system, we could try to strip them from the initial state
    // and hope that we are still "safe" to make further changes (to be determined by computeNewTopologyAndMetadata call below).
    const auto stripped_initial_state { win_utils::stripInitialState(new_state.m_initial, devices) };
    if (!stripped_initial_state) {
      // Error already logged
      return std::nullopt;
    }

    const auto &[new_topology, device_to_configure, additional_devices_to_configure] = win_utils::computeNewTopologyAndMetadata(config.m_device_prep, config.m_device_id, *stripped_initial_state);
    const auto change_is_needed { !m_dd_api->isTopologyTheSame(topology_before_changes, new_topology) };
    DD_LOG(info) << "Newly computed display device topology data:\n"
                 << "  - topology: " << toJson(new_topology, JSON_COMPACT) << "\n"
                 << "  - change is needed: " << toJson(change_is_needed, JSON_COMPACT) << "\n"
                 << "  - additional devices to configure: " << toJson(additional_devices_to_configure, JSON_COMPACT);

    // This check is mainly to cover the case for "config.device_prep == VerifyOnly" as we at least
    // have to validate that the device exists, but it doesn't hurt to double-check it in all cases.
    if (!win_utils::flattenTopology(new_topology).contains(device_to_configure)) {
      DD_LOG(error) << "Device " << toJson(device_to_configure, JSON_COMPACT) << " is not active!";
      return std::nullopt;
    }

    if (change_is_needed) {
      if (cached_state && !m_dd_api->isTopologyTheSame(cached_state->m_modified.m_topology, new_topology)) {
        DD_LOG(warning) << "To apply new display device settings, previous modifications must be undone! Trying to undo them now.";
        if (!revertModifiedSettings()) {
          DD_LOG(error) << "Failed to apply new configuration, because the previous settings could not be reverted!";
          return std::nullopt;
        }
      }

      const bool audio_is_captured { m_audio_context_api->isCaptured() };
      if (!audio_is_captured) {
        // Non-stripped initial state MUST be checked here as the missing device could have its context captured!
        const bool switching_from_initial { m_dd_api->isTopologyTheSame(new_state.m_initial.m_topology, topology_before_changes) };
        const bool new_topology_contains_all_current_topology_devices { std::ranges::includes(win_utils::flattenTopology(new_topology), win_utils::flattenTopology(topology_before_changes)) };
        if (switching_from_initial && !new_topology_contains_all_current_topology_devices) {
          // Only capture the context when switching from initial topology. All the other intermediate states, like non-existent
          // capture state after system restart are to be avoided.
          if (!m_audio_context_api->capture()) {
            DD_LOG(error) << "Failed to capture audio context!";
            return std::nullopt;
          }
        }
      }

      if (!m_dd_api->setTopology(new_topology)) {
        DD_LOG(error) << "Failed to apply new configuration, because a new topology could not be set!";
        return std::nullopt;
      }

      // We can release the context later on if everything is successful as we are switching back to the non-stripped initial state.
      release_context = m_dd_api->isTopologyTheSame(new_state.m_initial.m_topology, new_topology) && audio_is_captured;
    }

    new_state.m_modified.m_topology = new_topology;
    return std::make_tuple(new_state, device_to_configure, additional_devices_to_configure);
  }

  bool
  SettingsManager::preparePrimaryDevice(const SingleDisplayConfiguration &config, const std::string &device_to_configure, DdGuardFn &guard_fn, SingleDisplayConfigState &new_state) {
    const auto &cached_state { m_persistence_state->getState() };
    const auto cached_primary_device { cached_state ? cached_state->m_modified.m_original_primary_device : std::string {} };
    const bool ensure_primary { config.m_device_prep == SingleDisplayConfiguration::DevicePreparation::EnsurePrimary };
    const bool might_need_to_restore { !cached_primary_device.empty() };

    std::string current_primary_device;
    if (ensure_primary || might_need_to_restore) {
      current_primary_device = win_utils::getPrimaryDevice(*m_dd_api, new_state.m_modified.m_topology);
      if (current_primary_device.empty()) {
        DD_LOG(error) << "Failed to get primary device for the topology! Searched topology:\n"
                      << toJson(new_state.m_modified.m_topology);
        return false;
      }
    }

    const auto try_change { [&](const std::string &new_device, const auto info_preamble, const auto error_log) {
      if (current_primary_device != new_device) {
        DD_LOG(info) << info_preamble << toJson(new_device);
        if (!m_dd_api->setAsPrimary(new_device)) {
          DD_LOG(error) << error_log;
          return false;
        }

        guard_fn = win_utils::primaryGuardFn(*m_dd_api, current_primary_device);
      }

      return true;
    } };

    if (ensure_primary) {
      const auto original_primary_device { cached_primary_device.empty() ? current_primary_device : cached_primary_device };
      const auto &new_primary_device { device_to_configure };

      if (!try_change(new_primary_device, "Changing primary display to: ", "Failed to apply new configuration, because a new primary device could not be set!")) {
        // Error already logged
        return false;
      }

      // Here we preserve the data from persistence (unless there's none) as in the end that is what we want to go back to.
      new_state.m_modified.m_original_primary_device = original_primary_device;
      return true;
    }

    if (might_need_to_restore) {
      if (!try_change(cached_primary_device, "Changing primary display back to: ", "Failed to restore original primary device!")) {
        // Error already logged
        return false;
      }
    }

    return true;
  }
}  // namespace display_device
