/**
 * @file src/windows/settings_manager_revert.cpp
 * @brief Definitions for the methods for reverting settings in SettingsManager.
 */
// class header include
#include "display_device/windows/settings_manager.h"

// system includes
#include <algorithm>
#include <boost/scope/scope_exit.hpp>
#include <optional>
#include <ranges>

// local includes
#include "display_device/logging.h"
#include "display_device/windows/json.h"
#include "display_device/windows/settings_utils.h"

namespace display_device {
  namespace {
    /**
     * @brief Function that does nothing.
     */
    void noopFn() {
      // Intentionally empty guard callback.
    }

    /**
     * @brief Append non-empty groups from `source` containing only device IDs accepted by `keep_id`.
     * @tparam KeepFn Callable taking `const std::string &` and returning `bool`.
     * @param source Topology whose groups are filtered.
     * @param keep_id Predicate deciding whether a device ID should be included.
     * @param target Topology to append filtered groups into.
     */
    template<typename KeepFn>
    void appendFilteredTopologyGroups(const ActiveTopology &source, const KeepFn &keep_id, ActiveTopology &target) {
      for (const auto &group : source) {
        std::vector<std::string> remaining;
        for (const auto &id : group) {
          if (keep_id(id)) {
            remaining.push_back(id);
          }
        }
        if (!remaining.empty()) {
          target.push_back(std::move(remaining));
        }
      }
    }

    /**
     * @brief Append available groups without reactivating stream-only outputs or duplicating devices.
     * @param topology Groups to consider, in priority order.
     * @param available Currently enumerated device IDs.
     * @param activated Device IDs activated only for the stream.
     * @param target Recovery topology to extend.
     * @param included IDs already included in the recovery topology.
     */
    void appendRecoveryGroups(const ActiveTopology &topology, const StringSet &available, const StringSet &activated, ActiveTopology &target, StringSet &included) {
      appendFilteredTopologyGroups(
        topology,
        [&available, &activated, &included](const std::string &id) {
          return available.contains(id) && !activated.contains(id) && included.insert(id).second;
        },
        target
      );
    }

    /**
     * @brief Add the groups of a target topology to a staged topology, keeping clone groups intact.
     *
     * A target group that shares a device with a staged group extends that group in place instead of
     * being added separately. This way the staged topology needs no more sources than necessary.
     * Devices that are already staged stay where they are.
     * @param target Groups that must be active once staging is done.
     * @param staged Topology to extend.
     */
    void stageRecoveryGroups(const ActiveTopology &target, ActiveTopology &staged) {
      for (const auto &group : target) {
        const auto in_group {[&group](const auto &id) {
          return std::ranges::find(group, id) != group.end();
        }};
        const auto host {std::ranges::find_if(staged, [&in_group](const auto &staged_group) {
          return std::ranges::any_of(staged_group, in_group);
        })};
        if (host == staged.end()) {
          staged.push_back(group);
          continue;
        }

        std::vector<std::string> missing;
        for (const auto &id : group) {
          if (std::ranges::none_of(staged, [&id](const auto &staged_group) {
                return std::ranges::find(staged_group, id) != staged_group.end();
              })) {
            missing.push_back(id);
          }
        }
        if (host->size() + missing.size() > 2) {
          // Windows cannot clone more than two displays, so the extra members are staged on their own.
          if (!missing.empty()) {
            staged.push_back(std::move(missing));
          }
          continue;
        }

        host->insert(host->end(), missing.begin(), missing.end());
      }
    }

    /**
     * @brief Compare two topologies ignoring group and member order.
     * @param lhs First topology to compare.
     * @param rhs Second topology to compare.
     * @return True when both topologies contain the same clone groups.
     */
    bool areTopologiesEquivalent(ActiveTopology lhs, ActiveTopology rhs) {
      const auto normalize {[](ActiveTopology &topology) {
        for (auto &group : topology) {
          std::ranges::sort(group);
        }
        std::ranges::sort(topology);
      }};
      normalize(lhs);
      normalize(rhs);
      return lhs == rhs;
    }

    /**
     * @brief Collect all stream-only devices activated by the current or earlier pending session.
     * @param state Persisted state to inspect.
     * @return Device IDs activated only for streaming and excluded from recovery topologies.
     */
    StringSet collectActivatedDevices(const SingleDisplayConfigState &state) {
      const auto initial {win_utils::flattenTopology(state.m_initial.m_topology)};
      const auto modified {win_utils::flattenTopology(state.m_modified.m_topology)};
      StringSet activated {state.m_modified.m_activated_devices};
      std::ranges::set_difference(modified, initial, std::inserter(activated, activated.end()));
      for (const auto &id : initial) {
        activated.erase(id);
      }
      return activated;
    }

    /**
     * @brief Check whether every device in a topology is stream-only (absent from the initial topology).
     * @param topology Topology to inspect.
     * @param initial_ids Device IDs belonging to the initial topology.
     * @return True if `topology` is non-empty and none of its devices belong to `initial_ids`.
     */
    bool isStreamOnlyTopology(const ActiveTopology &topology, const StringSet &initial_ids) {
      const auto ids {win_utils::flattenTopology(topology)};
      return !ids.empty() && std::ranges::none_of(ids, [&initial_ids](const auto &id) {
        return initial_ids.contains(id);
      });
    }

    /**
     * @brief Remove stream-only devices that are no longer active from the modified state.
     *
     * A stream-only device (absent from the initial topology) can be deactivated externally by
     * Windows or HPD before or during revert. Dropping only inactive stream-only devices lets
     * revert finish restoring remaining active devices and return to the initial topology instead
     * of failing into full-extended mode.
     *
     * @param modified_state Modified state to prune in place.
     * @param initial_ids Device IDs belonging to the initial topology.
     * @param active_ids Device IDs currently active in the OS topology.
     * @return True if at least one inactive device was removed and none of the inactive devices belonged to `initial_ids`.
     */
    bool dropInactiveStreamOnlyDevices(SingleDisplayConfigState::Modified &modified_state, const StringSet &initial_ids, const StringSet &active_ids) {
      if (active_ids.empty()) {
        return false;
      }

      const auto modified_ids {win_utils::flattenTopology(modified_state.m_topology)};
      StringSet inactive_stream_only;
      for (const auto &id : modified_ids) {
        if (active_ids.contains(id)) {
          continue;
        }
        if (initial_ids.contains(id)) {
          return false;
        }
        inactive_stream_only.insert(id);
      }

      if (inactive_stream_only.empty()) {
        return false;
      }

      ActiveTopology remaining_topology;
      appendFilteredTopologyGroups(
        modified_state.m_topology,
        [&inactive_stream_only](const std::string &id) {
          return !inactive_stream_only.contains(id);
        },
        remaining_topology
      );
      modified_state.m_topology = std::move(remaining_topology);
      std::erase_if(modified_state.m_original_modes, [&inactive_stream_only](const auto &entry) {
        return inactive_stream_only.contains(entry.first);
      });
      std::erase_if(modified_state.m_original_hdr_states, [&inactive_stream_only](const auto &entry) {
        return inactive_stream_only.contains(entry.first);
      });
      if (inactive_stream_only.contains(modified_state.m_original_primary_device)) {
        modified_state.m_original_primary_device.clear();
      }
      if (inactive_stream_only.contains(modified_state.m_interim_primary_device)) {
        modified_state.m_interim_primary_device.clear();
      }
      return true;
    }

    /**
     * @brief Check whether an original device is away, so that its settings have to wait for its return.
     * @param id Device ID to check.
     * @param initial IDs of the devices in the initial topology.
     * @param available Currently enumerated device IDs.
     * @return True if the device is part of the initial topology and is unavailable.
     */
    bool isPendingDevice(const std::string &id, const StringSet &initial, const StringSet &available) {
      return initial.contains(id) && !available.contains(id);
    }

    /**
     * @brief Collect originally missing devices whose saved Windows database topology still needs restoration.
     *
     * Tracks only the originally missing displays so a fallback panel added during recovery does not
     * stay pending if the laptop lid is closed again before redocking.
     * @param state Persisted state to inspect.
     * @param initial Device IDs in the initial topology.
     * @param available Currently enumerated device IDs.
     * @param keep_pending_topology True when Windows' display database was updated and needs restoration on return.
     * @return Pending topology device IDs that are still unavailable.
     */
    StringSet collectPendingTopologyDevices(const SingleDisplayConfigState &state, const StringSet &initial, const StringSet &available, const bool keep_pending_topology) {
      if (!keep_pending_topology) {
        return {};
      }

      const auto &tracked_topology_devices {
        state.m_modified.m_pending_topology_devices.empty() ? initial : state.m_modified.m_pending_topology_devices
      };
      StringSet pending;
      for (const auto &id : tracked_topology_devices) {
        if (isPendingDevice(id, initial, available)) {
          pending.insert(id);
        }
      }
      return pending;
    }

    /**
     * @brief Keep only the pending settings of original devices that are currently unavailable.
     *
     * These settings cannot be restored until the devices return, so they must outlive a topology recovery.
     * When CCD writes update Windows' display database, original devices that are still unavailable also
     * retain a pending topology record so their saved database layout is restored once they return.
     * The modified topology is reduced to those devices, as they are all that is needed to restore the settings.
     * The initial topology is extended with the recovered one, so that new settings can still be applied while the devices are away.
     * @param state State to filter.
     * @param available Currently enumerated device IDs.
     * @param recovered_topology Topology that replaced the original one.
     * @param keep_pending_topology True when Windows' display database was updated and needs restoration on return.
     * @return State with only the settings of unavailable original devices, or nothing if there are none.
     */
    std::optional<SingleDisplayConfigState> keepUnavailableSettings(const SingleDisplayConfigState &state, const StringSet &available, const ActiveTopology &recovered_topology, const bool keep_pending_topology) {
      const auto initial {win_utils::flattenTopology(state.m_initial.m_topology)};
      const auto activated {collectActivatedDevices(state)};

      SingleDisplayConfigState result {state};
      std::erase_if(result.m_modified.m_original_modes, [&initial, &available](const auto &entry) {
        return !isPendingDevice(entry.first, initial, available);
      });
      std::erase_if(result.m_modified.m_original_hdr_states, [&initial, &available](const auto &entry) {
        return !isPendingDevice(entry.first, initial, available);
      });
      if (!isPendingDevice(result.m_modified.m_original_primary_device, initial, available)) {
        result.m_modified.m_original_primary_device.clear();
      }
      result.m_modified.m_interim_primary_device.clear();  // Already restored by the revert that led here.
      result.m_modified.m_pending_topology_devices = collectPendingTopologyDevices(state, initial, available, keep_pending_topology);

      if (!result.m_modified.hasModifications() && result.m_modified.m_pending_topology_devices.empty()) {
        return std::nullopt;
      }

      result.m_modified.m_activated_devices = activated;

      // Only the returning devices need to be active to restore their settings.
      result.m_modified.m_topology.clear();
      StringSet included;
      appendFilteredTopologyGroups(
        state.m_initial.m_topology,
        [&initial, &available, &included](const std::string &id) {
          return isPendingDevice(id, initial, available) && included.insert(id).second;
        },
        result.m_modified.m_topology
      );

      // The original devices alone would leave nothing to build on until one of them returns.
      StringSet known {initial};
      appendFilteredTopologyGroups(
        recovered_topology,
        [&known](const std::string &id) {
          return known.insert(id).second;
        },
        result.m_initial.m_topology
      );
      return result;
    }

    /**
     * @brief Find the original devices that have settings recorded but are left out of the modified topology.
     *
     * These are carried over from an earlier session, e.g. the settings of a display that was unplugged at the time.
     * @param state State to inspect.
     * @return IDs of the devices that are in the initial topology only.
     */
    StringSet findCarriedDevices(const SingleDisplayConfigState &state) {
      const auto &modified {state.m_modified};
      StringSet recorded;
      for (const auto &id : modified.m_original_modes | std::views::keys) {
        recorded.insert(id);
      }
      for (const auto &id : modified.m_original_hdr_states | std::views::keys) {
        recorded.insert(id);
      }
      recorded.insert(modified.m_original_primary_device);

      const auto initial_ids {win_utils::flattenTopology(state.m_initial.m_topology)};
      const auto topology_ids {win_utils::flattenTopology(modified.m_topology)};
      StringSet carried;
      for (const auto &id : recorded) {
        if (initial_ids.contains(id) && !topology_ids.contains(id)) {
          carried.insert(id);
        }
      }
      return carried;
    }

    /**
     * @brief Restrict modified settings to the devices that are still available.
     * @param modified Modified settings to filter.
     * @param available Currently enumerated device IDs.
     * @return Modified settings without the unavailable devices.
     */
    SingleDisplayConfigState::Modified keepAvailableDevices(const SingleDisplayConfigState::Modified &modified, const StringSet &available) {
      SingleDisplayConfigState::Modified result {modified};
      result.m_topology.clear();
      StringSet included;
      appendRecoveryGroups(modified.m_topology, available, {}, result.m_topology, included);
      std::erase_if(result.m_original_modes, [&available](const auto &entry) {
        return !available.contains(entry.first);
      });
      std::erase_if(result.m_original_hdr_states, [&available](const auto &entry) {
        return !available.contains(entry.first);
      });
      if (!available.contains(result.m_original_primary_device)) {
        // The interim device is what was primary before the original one went away.
        result.m_original_primary_device = available.contains(result.m_interim_primary_device) ? result.m_interim_primary_device : std::string {};
      }
      result.m_interim_primary_device.clear();
      return result;
    }

    /**
     * @brief Activate the carried devices that are back and drop the settings of those that are not.
     * @param dd_api Display device API.
     * @param state Persisted state that carries the settings.
     * @param modified_state Modified settings to prepare for the revert.
     * @return True if some carried devices are still unavailable, so their settings have to be kept.
     */
    bool prepareCarriedDevices(const WinDisplayDeviceInterface &dd_api, const SingleDisplayConfigState &state, SingleDisplayConfigState::Modified &modified_state) {
      const auto carried {findCarriedDevices(state)};
      if (carried.empty()) {
        return false;
      }

      // Settings carried over from an earlier session can only be restored once their devices are active.
      const auto available {win_utils::getDeviceIds(dd_api.enumAvailableDevices())};
      if (available.empty()) {
        return false;
      }

      bool some_unavailable {false};
      for (const auto &id : carried) {
        if (available.contains(id)) {
          modified_state.m_topology.push_back({id});
        } else {
          some_unavailable = true;
        }
      }
      modified_state = keepAvailableDevices(modified_state, available);
      return some_unavailable;
    }

    /**
     * @brief Restore a safe fallback topology when `revertSettings()` fails midway.
     *
     * When the desktop already matched the original layout before revert started, restore that layout
     * (only if revert switched away) instead of force-enabling all available displays into Extended mode.
     * @param dd_api Display device API.
     * @param current_topology Active topology observed when `revertSettings()` started.
     * @param initial_topology Persisted original topology.
     * @param switched_to_modified_topology True if revert switched away from `current_topology`.
     * @param system_settings_touched Updated to true when a topology write is attempted.
     */
    void restoreTopologyOnRevertFailure(
      WinDisplayDeviceInterface &dd_api,
      const ActiveTopology &current_topology,
      const ActiveTopology &initial_topology,
      const bool switched_to_modified_topology,
      bool &system_settings_touched
    ) {
      if (areTopologiesEquivalent(current_topology, initial_topology)) {
        if (switched_to_modified_topology) {
          system_settings_touched = true;
          if (!dd_api.setTopology(current_topology)) {
            DD_LOG(error) << "failed to revert topology in revertSettings topology guard! Used the following topology:\n"
                          << toJson(current_topology);
          }
        }
        return;
      }

      auto topology_to_restore {win_utils::createFullExtendedTopology(dd_api)};
      if (!dd_api.isTopologyValid(topology_to_restore)) {
        topology_to_restore = current_topology;
      }

      const bool is_topology_the_same {dd_api.isTopologyTheSame(current_topology, topology_to_restore)};
      system_settings_touched = system_settings_touched || !is_topology_the_same;
      if (!is_topology_the_same && !dd_api.setTopology(topology_to_restore)) {
        DD_LOG(error) << "failed to revert topology in revertSettings topology guard! Used the following topology:\n"
                      << toJson(topology_to_restore);
      }
    }

    /**
     * @brief Clear transient modified settings fields while keeping pending recovery metadata intact.
     * @param modified_state Modified state whose per-session fields should be cleared.
     */
    void clearTransientModifications(SingleDisplayConfigState::Modified &modified_state) {
      modified_state.m_topology.clear();
      modified_state.m_original_modes.clear();
      modified_state.m_original_hdr_states.clear();
      modified_state.m_original_primary_device.clear();
      modified_state.m_interim_primary_device.clear();
    }

    /**
     * @brief Working state produced while preparing the modified topology for settings revert.
     */
    struct ModifiedTopologyState {
      SingleDisplayConfigState::Modified m_modified {};  ///< Modified state adjusted for available devices.
      bool m_keep_record {false};  ///< True when missing devices require retaining the persisted record.
      bool m_switched_topology {false};  ///< True when the OS topology was switched away from `current_topology`.
    };

    /**
     * @brief Recover when switching to the modified topology fails during `revertModifiedSettings()`.
     * @param dd_api Display device API.
     * @param current_topology Current active topology before revert.
     * @param initial_ids Device IDs belonging to the initial topology.
     * @param state Working modified topology state updated in place.
     * @return An early revert result to return immediately, or `std::nullopt` to continue reverting settings.
     */
    std::optional<SettingsManager::RevertResult> recoverModifiedTopologySwitchFailure(
      WinDisplayDeviceInterface &dd_api,
      const ActiveTopology &current_topology,
      const StringSet &initial_ids,
      ModifiedTopologyState &state
    ) {
      DD_LOG(error) << "Failed to change topology to:\n"
                    << toJson(state.m_modified.m_topology);

      const auto available {win_utils::getDeviceIds(dd_api.enumAvailableDevices())};
      const auto modified_ids {win_utils::flattenTopology(state.m_modified.m_topology)};
      const bool devices_missing {!available.empty() && std::ranges::any_of(modified_ids, [&available](const auto &id) {
        return !available.contains(id);
      })};
      if (!devices_missing) {
        if (!isStreamOnlyTopology(state.m_modified.m_topology, initial_ids)) {
          return SettingsManager::RevertResult::SwitchingTopologyFailed;
        }

        DD_LOG(warning) << "Stream-only modified topology could not be reactivated; skipping modified settings restoration.";
        clearTransientModifications(state.m_modified);
        state.m_switched_topology = false;
        return std::nullopt;
      }

      state.m_keep_record = true;

      // A display from the modified topology has been unplugged. Revert the settings of the remaining
      // displays so that the initial topology, or its recovery, can still be applied.
      state.m_modified = keepAvailableDevices(state.m_modified, available);
      DD_LOG(warning) << "Some modified devices are unavailable, reverting settings for the remaining topology:\n"
                      << toJson(state.m_modified.m_topology);
      if (state.m_modified.m_topology.empty()) {
        return SettingsManager::RevertResult::Ok;
      }

      const bool is_topology_the_same {dd_api.isTopologyTheSame(current_topology, state.m_modified.m_topology)};
      if (!is_topology_the_same && !dd_api.setTopology(state.m_modified.m_topology)) {
        DD_LOG(error) << "Failed to change topology to:\n"
                      << toJson(state.m_modified.m_topology);
        return SettingsManager::RevertResult::SwitchingTopologyFailed;
      }

      state.m_switched_topology = !is_topology_the_same;
      return std::nullopt;
    }

    /**
     * @brief Prune inactive stream-only devices from `modified_state` when allowed.
     * @param dd_api Display device API.
     * @param modified_state Modified state to prune in place.
     * @param initial_ids Device IDs belonging to the initial topology.
     * @param allow_pruning True when pruning inactive stream-only devices is permitted for this failure.
     * @return True if one or more inactive stream-only devices were pruned.
     */
    bool tryPruneInactiveStreamOnlyDevices(
      const WinDisplayDeviceInterface &dd_api,
      SingleDisplayConfigState::Modified &modified_state,
      const StringSet &initial_ids,
      const bool allow_pruning
    ) {
      if (!allow_pruning) {
        return false;
      }

      if (const auto active_ids {win_utils::flattenTopology(dd_api.getCurrentTopology())}; !dropInactiveStreamOnlyDevices(modified_state, initial_ids, active_ids)) {
        return false;
      }

      DD_LOG(warning) << "Some stream-only modified devices are no longer active; skipping remaining settings restoration for them.";
      return true;
    }

    /**
     * @brief Run a modified-settings revert step, pruning inactive stream-only devices and retrying if needed.
     * @tparam RevertFn Callable taking `bool *query_failed` and returning `SettingsManager::RevertResult`.
     * @tparam DisarmGuardsFn Callable disarming rollback guards when all modified devices were pruned.
     * @param dd_api Display device API.
     * @param modified_state Modified state being reverted.
     * @param initial_ids Device IDs belonging to the initial topology.
     * @param revert_fn Step function to execute.
     * @param disarm_guards Callback invoked if pruning leaves no modified devices to revert.
     * @return Result of the step or retry.
     */
    template<typename RevertFn, typename DisarmGuardsFn>
    SettingsManager::RevertResult revertStepWithPruning(
      const WinDisplayDeviceInterface &dd_api,
      SingleDisplayConfigState::Modified &modified_state,
      const StringSet &initial_ids,
      const RevertFn &revert_fn,
      const DisarmGuardsFn &disarm_guards
    ) {
      bool query_failed {false};
      const auto first_result {revert_fn(&query_failed)};
      if (first_result == SettingsManager::RevertResult::Ok) {
        return first_result;
      }

      const bool allow_pruning {query_failed || isStreamOnlyTopology(modified_state.m_topology, initial_ids)};
      if (!tryPruneInactiveStreamOnlyDevices(dd_api, modified_state, initial_ids, allow_pruning)) {
        // Error already logged
        return first_result;
      }

      if (modified_state.m_topology.empty()) {
        // All modified devices were stream-only and are already inactive; avoid invoking guards on inactive paths.
        disarm_guards();
        return SettingsManager::RevertResult::Ok;
      }

      return revert_fn(nullptr);
    }
  }  // namespace

  SettingsManager::RevertResult SettingsManager::revertSettings() {
    const auto &cached_state {m_persistence_state->getState()};
    if (!cached_state) {
      return RevertResult::Ok;
    }

    const auto api_access {m_dd_api->isApiAccessAvailable()};
    DD_LOG(info) << "Trying to revert applied display device settings. API is available: " << toJson(api_access);

    if (!api_access) {
      return RevertResult::ApiTemporarilyUnavailable;
    }

    const auto current_topology {m_dd_api->getCurrentTopology()};
    if (!m_dd_api->isTopologyValid(current_topology)) {
      DD_LOG(error) << "Retrieved current topology is invalid:\n"
                    << toJson(current_topology);
      return RevertResult::TopologyIsInvalid;
    }

    const ActiveTopology initial_topology {cached_state->m_initial.m_topology};
    bool system_settings_touched {false};
    bool switched_to_modified_topology {false};
    boost::scope::scope_exit hdr_blank_always_executed_guard {[this, &system_settings_touched]() {
      if (system_settings_touched) {
        win_utils::blankHdrStates(*m_dd_api, m_workarounds.m_hdr_blank_delay);
      }
    }};
    boost::scope::scope_exit topology_prep_guard {[this, &current_topology, &initial_topology, &system_settings_touched, &switched_to_modified_topology]() {
      restoreTopologyOnRevertFailure(*m_dd_api, current_topology, initial_topology, switched_to_modified_topology, system_settings_touched);
    }};

    // We can revert the modified setting independently before playing around with initial topology.
    if (const auto result = revertModifiedSettings(current_topology, system_settings_touched, &switched_to_modified_topology); result != RevertResult::Ok) {
      // Error already logged
      return result;
    }

    if (!m_dd_api->isTopologyValid(cached_state->m_initial.m_topology)) {
      DD_LOG(error) << "Trying to revert to an invalid initial topology:\n"
                    << toJson(cached_state->m_initial.m_topology);
      return RevertResult::TopologyIsInvalid;
    }

    const bool is_topology_the_same {m_dd_api->isTopologyTheSame(current_topology, cached_state->m_initial.m_topology)};
    const bool need_to_switch_topology {!is_topology_the_same || switched_to_modified_topology};
    system_settings_touched = system_settings_touched || !is_topology_the_same;
    std::optional<SingleDisplayConfigState> pending_state;
    if (need_to_switch_topology && !m_dd_api->setTopology(cached_state->m_initial.m_topology)) {
      DD_LOG(error) << "Failed to change topology to:\n"
                    << toJson(cached_state->m_initial.m_topology);
      if (!recoverMissingTopology(current_topology)) {
        return RevertResult::SwitchingTopologyFailed;
      }

      // Original devices that are still unplugged keep their settings (and saved database topology, when enabled) until they return.
      pending_state = keepUnavailableSettings(*cached_state, win_utils::getDeviceIds(m_dd_api->enumAvailableDevices()), m_dd_api->getCurrentTopology(), m_dd_api->savesToDatabase());
    }

    if (!m_persistence_state->persistState(pending_state)) {
      DD_LOG(error) << "Failed to save reverted settings! Undoing initial topology changes...";
      return RevertResult::PersistenceSaveFailed;
    }

    if (m_audio_context_api->isCaptured()) {
      m_audio_context_api->release();
    }

    // Disable guards
    topology_prep_guard.set_active(false);
    return RevertResult::Ok;
  }

  bool SettingsManager::recoverMissingTopology(const ActiveTopology &current_topology) {
    const auto &state {*m_persistence_state->getState()};
    const auto initial {win_utils::flattenTopology(state.m_initial.m_topology)};
    const auto devices {m_dd_api->enumAvailableDevices()};
    const auto available {win_utils::getDeviceIds(devices)};
    if (available.empty() || std::ranges::all_of(initial, [&available](const auto &id) {
          return available.contains(id);
        })) {
      // An API failure with unchanged hardware must not discard the original state.
      return false;
    }

    const auto activated {collectActivatedDevices(state)};
    ActiveTopology target;
    StringSet included;
    appendRecoveryGroups(state.m_initial.m_topology, available, activated, target, included);
    if (target.empty()) {
      // An arbitrary virtual/unknown output is not proof of a usable laptop screen.
      for (const auto &device : devices) {
        if (device.m_is_internal && !activated.contains(device.m_device_id)) {
          appendRecoveryGroups({{device.m_device_id}}, available, activated, target, included);
          break;
        }
      }
    }
    if (target.empty()) {
      return false;  // Closed lid and no original display: retain pending recovery.
    }
    const auto replacements {win_utils::flattenTopology(target)};
    appendRecoveryGroups(current_topology, available, activated, target, included);  // Preserve unrelated displays activated by the user.

    // First activate the replacement without switching off the current output.
    // Keep the target's groups intact, even when a member is already active, since cloned displays share a source.
    ActiveTopology staged {current_topology};
    stageRecoveryGroups(target, staged);
    if (!m_dd_api->setTopology(staged)) {
      return false;
    }
    if (const auto active {win_utils::flattenTopology(m_dd_api->getCurrentTopology())}; !std::ranges::all_of(replacements, [&active](const auto &id) {
          return active.contains(id);
        })) {
      return false;
    }
    if (!m_dd_api->setTopology(target) || !m_dd_api->isTopologyTheSame(m_dd_api->getCurrentTopology(), target)) {
      return false;
    }
    DD_LOG(info) << "Recovered display topology after original devices disappeared:\n"
                 << toJson(target);
    return true;
  }

  SettingsManager::RevertResult SettingsManager::revertModifiedHdrStates(const SingleDisplayConfigState::Modified &modified_state, DdGuardFn &guard_fn, bool &system_settings_touched, bool *query_failed) {
    using enum RevertResult;

    if (modified_state.m_original_hdr_states.empty()) {
      return Ok;
    }

    const auto current_states {m_dd_api->getCurrentHdrStates(win_utils::flattenTopology(modified_state.m_topology))};
    if (current_states.empty()) {
      if (query_failed) {
        *query_failed = true;
      }
      DD_LOG(error) << "Failed to get current HDR states!";
      return RevertingHdrStatesFailed;
    }

    if (current_states == modified_state.m_original_hdr_states) {
      return Ok;
    }

    system_settings_touched = true;

    DD_LOG(info) << "Trying to change back the HDR states to:\n"
                 << toJson(modified_state.m_original_hdr_states);
    if (!m_dd_api->setHdrStates(modified_state.m_original_hdr_states)) {
      // Error already logged
      return RevertingHdrStatesFailed;
    }

    guard_fn = win_utils::hdrStateGuardFn(*m_dd_api, current_states);
    return Ok;
  }

  SettingsManager::RevertResult SettingsManager::revertModifiedDisplayModes(const SingleDisplayConfigState::Modified &modified_state, DdGuardFn &guard_fn, bool &system_settings_touched, bool *query_failed) {
    using enum RevertResult;

    if (modified_state.m_original_modes.empty()) {
      return Ok;
    }

    const auto current_modes {m_dd_api->getCurrentDisplayModes(win_utils::flattenTopology(modified_state.m_topology))};
    if (current_modes.empty()) {
      if (query_failed) {
        *query_failed = true;
      }
      DD_LOG(error) << "Failed to get current display modes!";
      return RevertingDisplayModesFailed;
    }

    if (current_modes == modified_state.m_original_modes) {
      return Ok;
    }

    DD_LOG(info) << "Trying to change back the display modes to:\n"
                 << toJson(modified_state.m_original_modes);
    if (!m_dd_api->setDisplayModes(modified_state.m_original_modes)) {
      system_settings_touched = true;
      // Error already logged
      return RevertingDisplayModesFailed;
    }

    // It is possible that the display modes will not actually change even though the "current != new" condition is true.
    // This is because of some additional internal checks that determine whether the change is actually needed.
    // Therefore, we should check the current display modes after the fact!
    if (current_modes != m_dd_api->getCurrentDisplayModes(win_utils::flattenTopology(modified_state.m_topology))) {
      system_settings_touched = true;
      guard_fn = win_utils::modeGuardFn(*m_dd_api, current_modes);
    }

    return Ok;
  }

  SettingsManager::RevertResult SettingsManager::revertModifiedPrimaryDevice(const SingleDisplayConfigState::Modified &modified_state, DdGuardFn &guard_fn, bool &system_settings_touched) {
    using enum RevertResult;

    if (modified_state.m_original_primary_device.empty()) {
      return Ok;
    }

    const auto current_primary_device {win_utils::getPrimaryDevice(*m_dd_api, modified_state.m_topology)};
    if (current_primary_device == modified_state.m_original_primary_device) {
      return Ok;
    }

    system_settings_touched = true;

    DD_LOG(info) << "Trying to change back the original primary device to: " << toJson(modified_state.m_original_primary_device);
    if (!m_dd_api->setAsPrimary(modified_state.m_original_primary_device)) {
      // Error already logged
      return RevertingPrimaryDeviceFailed;
    }

    guard_fn = win_utils::primaryGuardFn(*m_dd_api, current_primary_device);
    return Ok;
  }

  SettingsManager::RevertResult SettingsManager::revertModifiedSettings(const ActiveTopology &current_topology, bool &system_settings_touched, bool *switched_topology) {
    const auto &cached_state {m_persistence_state->getState()};
    if (!cached_state || !cached_state->m_modified.hasModifications()) {
      return RevertResult::Ok;
    }

    if (!m_dd_api->isTopologyValid(cached_state->m_modified.m_topology)) {
      DD_LOG(error) << "Trying to revert modified settings using invalid topology:\n"
                    << toJson(cached_state->m_modified.m_topology);
      return RevertResult::TopologyIsInvalid;
    }

    const auto initial_ids {win_utils::flattenTopology(cached_state->m_initial.m_topology)};
    ModifiedTopologyState state {cached_state->m_modified};
    state.m_keep_record = prepareCarriedDevices(*m_dd_api, *cached_state, state.m_modified);
    const bool is_topology_the_same {m_dd_api->isTopologyTheSame(current_topology, state.m_modified.m_topology)};
    system_settings_touched = !is_topology_the_same;
    state.m_switched_topology = !is_topology_the_same;
    if (!is_topology_the_same && !m_dd_api->setTopology(state.m_modified.m_topology)) {
      if (const auto early_result {recoverModifiedTopologySwitchFailure(*m_dd_api, current_topology, initial_ids, state)}; early_result.has_value()) {
        return *early_result;
      }
    }
    if (switched_topology) {
      *switched_topology = state.m_switched_topology;
    }

    auto &modified_state {state.m_modified};
    DdGuardFn hdr_guard_fn {noopFn};
    boost::scope::scope_exit<DdGuardFn &> hdr_guard {hdr_guard_fn};
    DdGuardFn mode_guard_fn {noopFn};
    boost::scope::scope_exit<DdGuardFn &> mode_guard {mode_guard_fn};
    DdGuardFn primary_guard_fn {noopFn};
    boost::scope::scope_exit<DdGuardFn &> primary_guard {primary_guard_fn};
    const auto disarm_all_guards {[&hdr_guard, &mode_guard, &primary_guard]() {
      hdr_guard.set_active(false);
      mode_guard.set_active(false);
      primary_guard.set_active(false);
    }};

    const auto revert_hdr {[this, &modified_state, &hdr_guard_fn, &system_settings_touched](bool *query_failed) {
      return revertModifiedHdrStates(modified_state, hdr_guard_fn, system_settings_touched, query_failed);
    }};
    if (const auto result {revertStepWithPruning(*m_dd_api, modified_state, initial_ids, revert_hdr, disarm_all_guards)}; result != RevertResult::Ok) {
      return result;
    }

    const auto revert_modes {[this, &modified_state, &mode_guard_fn, &system_settings_touched](bool *query_failed) {
      return revertModifiedDisplayModes(modified_state, mode_guard_fn, system_settings_touched, query_failed);
    }};
    if (const auto result {revertStepWithPruning(*m_dd_api, modified_state, initial_ids, revert_modes, disarm_all_guards)}; result != RevertResult::Ok) {
      return result;
    }

    const auto revert_primary {[this, &modified_state, &primary_guard_fn, &system_settings_touched](bool *) {
      return revertModifiedPrimaryDevice(modified_state, primary_guard_fn, system_settings_touched);
    }};
    if (const auto result {revertStepWithPruning(*m_dd_api, modified_state, initial_ids, revert_primary, disarm_all_guards)}; result != RevertResult::Ok) {
      return result;
    }

    if (state.m_keep_record) {
      // Keep the full record so that the unplugged displays can still be reverted if they return.
      disarm_all_guards();
      return RevertResult::Ok;
    }

    auto cleared_data {*cached_state};
    cleared_data.m_modified = {cleared_data.m_modified.m_topology};
    cleared_data.m_modified.m_pending_topology_devices = cached_state->m_modified.m_pending_topology_devices;
    cleared_data.m_modified.m_activated_devices = cached_state->m_modified.m_activated_devices;
    if (!m_persistence_state->persistState(cleared_data)) {
      DD_LOG(error) << "Failed to save reverted settings! Undoing changes to modified topology...";
      return RevertResult::PersistenceSaveFailed;
    }

    disarm_all_guards();
    return RevertResult::Ok;
  }
}  // namespace display_device
