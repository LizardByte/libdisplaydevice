/**
 * @file src/windows/settings_manager_revert.cpp
 * @brief Definitions for the methods for reverting settings in SettingsManager.
 */
// class header include
#include "display_device/windows/settings_manager.h"

// system includes
#include <algorithm>
#include <boost/scope/scope_exit.hpp>

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

    bool system_settings_touched {false};
    boost::scope::scope_exit hdr_blank_always_executed_guard {[this, &system_settings_touched]() {
      if (system_settings_touched) {
        win_utils::blankHdrStates(*m_dd_api, m_workarounds.m_hdr_blank_delay);
      }
    }};
    boost::scope::scope_exit topology_prep_guard {[this, &current_topology, &system_settings_touched]() {
      auto topology_to_restore {win_utils::createFullExtendedTopology(*m_dd_api)};
      if (!m_dd_api->isTopologyValid(topology_to_restore)) {
        topology_to_restore = current_topology;
      }

      const bool is_topology_the_same {m_dd_api->isTopologyTheSame(current_topology, topology_to_restore)};
      system_settings_touched = system_settings_touched || !is_topology_the_same;
      if (!is_topology_the_same && !m_dd_api->setTopology(topology_to_restore)) {
        DD_LOG(error) << "failed to revert topology in revertSettings topology guard! Used the following topology:\n"
                      << toJson(topology_to_restore);
      }
    }};

    // We can revert the modified setting independently before playing around with initial topology.
    bool switched_to_modified_topology {false};
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
    if (need_to_switch_topology && !m_dd_api->setTopology(cached_state->m_initial.m_topology)) {
      DD_LOG(error) << "Failed to change topology to:\n"
                    << toJson(cached_state->m_initial.m_topology);
      if (!recoverMissingTopology(current_topology)) {
        return RevertResult::SwitchingTopologyFailed;
      }
    }

    if (!m_persistence_state->persistState(std::nullopt)) {
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
    const auto modified {win_utils::flattenTopology(state.m_modified.m_topology)};
    const auto devices {m_dd_api->enumAvailableDevices()};
    StringSet available;
    for (const auto &device : devices) {
      available.insert(device.m_device_id);
    }
    if (available.empty() || std::ranges::all_of(initial, [&](const auto &id) {
          return available.contains(id);
        })) {
      // An API failure with unchanged hardware must not discard the original state.
      return false;
    }

    StringSet activated;
    std::ranges::set_difference(modified, initial, std::inserter(activated, activated.end()));
    ActiveTopology target;
    StringSet included;
    const auto append = [&](const ActiveTopology &topology) {
      for (const auto &group : topology) {
        std::vector<std::string> remaining;
        for (const auto &id : group) {
          if (available.contains(id) && !activated.contains(id) && included.insert(id).second) {
            remaining.push_back(id);
          }
        }
        if (!remaining.empty()) {
          target.push_back(std::move(remaining));
        }
      }
    };
    append(state.m_initial.m_topology);
    if (target.empty()) {
      // An arbitrary virtual/unknown output is not proof of a usable laptop screen.
      for (const auto &device : devices) {
        if (device.m_is_internal && !activated.contains(device.m_device_id)) {
          append({{device.m_device_id}});
          break;
        }
      }
    }
    if (target.empty()) {
      return false;  // Closed lid and no original display: retain pending recovery.
    }
    const auto replacements {win_utils::flattenTopology(target)};
    append(current_topology);  // Preserve unrelated displays activated by the user.

    // First activate the replacement without switching off the current output.
    ActiveTopology staged {current_topology};
    auto staged_ids {win_utils::flattenTopology(staged)};
    for (const auto &id : included) {
      if (staged_ids.insert(id).second) {
        staged.push_back({id});
      }
    }
    if (!m_dd_api->setTopology(staged)) {
      return false;
    }
    const auto active {win_utils::flattenTopology(m_dd_api->getCurrentTopology())};
    if (!std::ranges::all_of(replacements, [&](const auto &id) {
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

  SettingsManager::RevertResult SettingsManager::revertModifiedHdrStates(const SingleDisplayConfigState::Modified &modified_state, DdGuardFn &guard_fn, bool &system_settings_touched) {
    using enum RevertResult;

    if (modified_state.m_original_hdr_states.empty()) {
      return Ok;
    }

    const auto current_states {m_dd_api->getCurrentHdrStates(win_utils::flattenTopology(modified_state.m_topology))};
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

  SettingsManager::RevertResult SettingsManager::revertModifiedDisplayModes(const SingleDisplayConfigState::Modified &modified_state, DdGuardFn &guard_fn, bool &system_settings_touched) {
    using enum RevertResult;

    if (modified_state.m_original_modes.empty()) {
      return Ok;
    }

    const auto current_modes {m_dd_api->getCurrentDisplayModes(win_utils::flattenTopology(modified_state.m_topology))};
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

    const bool is_topology_the_same {m_dd_api->isTopologyTheSame(current_topology, cached_state->m_modified.m_topology)};
    system_settings_touched = !is_topology_the_same;
    if (!is_topology_the_same && !m_dd_api->setTopology(cached_state->m_modified.m_topology)) {
      DD_LOG(error) << "Failed to change topology to:\n"
                    << toJson(cached_state->m_modified.m_topology);
      return RevertResult::SwitchingTopologyFailed;
    }
    if (switched_topology) {
      *switched_topology = !is_topology_the_same;
    }

    DdGuardFn hdr_guard_fn {noopFn};
    boost::scope::scope_exit<DdGuardFn &> hdr_guard {hdr_guard_fn};
    if (const auto result {revertModifiedHdrStates(cached_state->m_modified, hdr_guard_fn, system_settings_touched)}; result != RevertResult::Ok) {
      // Error already logged
      return result;
    }

    DdGuardFn mode_guard_fn {noopFn};
    boost::scope::scope_exit<DdGuardFn &> mode_guard {mode_guard_fn};
    if (const auto result {revertModifiedDisplayModes(cached_state->m_modified, mode_guard_fn, system_settings_touched)}; result != RevertResult::Ok) {
      // Error already logged
      return result;
    }

    DdGuardFn primary_guard_fn {noopFn};
    boost::scope::scope_exit<DdGuardFn &> primary_guard {primary_guard_fn};
    if (const auto result {revertModifiedPrimaryDevice(cached_state->m_modified, primary_guard_fn, system_settings_touched)}; result != RevertResult::Ok) {
      // Error already logged
      return result;
    }

    auto cleared_data {*cached_state};
    cleared_data.m_modified = {cleared_data.m_modified.m_topology};
    if (!m_persistence_state->persistState(cleared_data)) {
      DD_LOG(error) << "Failed to save reverted settings! Undoing changes to modified topology...";
      return RevertResult::PersistenceSaveFailed;
    }

    // Disable guards
    hdr_guard.set_active(false);
    mode_guard.set_active(false);
    primary_guard.set_active(false);
    return RevertResult::Ok;
  }
}  // namespace display_device
