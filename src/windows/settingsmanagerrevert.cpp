// class header include
#include "displaydevice/windows/settingsmanager.h"

// system includes
#include <boost/scope/scope_exit.hpp>

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/windows/json.h"
#include "displaydevice/windows/settingsutils.h"

namespace display_device {
  bool
  SettingsManager::revertSettings() {
    const auto &cached_state { m_persistence_state->getState() };
    if (!cached_state) {
      return true;
    }

    const auto api_access { m_dd_api->isApiAccessAvailable() };
    DD_LOG(info) << "Trying to revert applied display device settings. API is available: " << toJson(api_access);

    if (!api_access) {
      return false;
    }

    const auto current_topology { m_dd_api->getCurrentTopology() };
    if (!m_dd_api->isTopologyValid(current_topology)) {
      DD_LOG(error) << "Retrieved current topology is invalid:\n"
                    << toJson(current_topology);
      return false;
    }

    boost::scope::scope_exit topology_prep_guard { win_utils::topologyGuardFn(*m_dd_api, current_topology) };

    // We can revert the modified setting independently before playing around with initial topology.
    if (!revertModifiedSettings()) {
      // Error already logged
      return false;
    }

    if (!m_dd_api->isTopologyValid(cached_state->m_initial.m_topology)) {
      DD_LOG(error) << "Trying to revert to an invalid initial topology:\n"
                    << toJson(cached_state->m_initial.m_topology);
      return false;
    }

    if (!m_dd_api->setTopology(cached_state->m_initial.m_topology)) {
      DD_LOG(error) << "Failed to change topology to:\n"
                    << toJson(cached_state->m_initial.m_topology);
      return false;
    }

    if (!m_persistence_state->persistState(std::nullopt)) {
      DD_LOG(error) << "Failed to save reverted settings! Undoing initial topology changes...";
      return false;
    }

    if (m_audio_context_api->isCaptured()) {
      m_audio_context_api->release();
    }

    // Disable guards
    topology_prep_guard.set_active(false);
    return true;
  }

  bool
  SettingsManager::revertModifiedSettings() {
    const auto &cached_state { m_persistence_state->getState() };
    if (!cached_state || !cached_state->m_modified.hasModifications()) {
      return true;
    }

    if (!m_dd_api->isTopologyValid(cached_state->m_modified.m_topology)) {
      DD_LOG(error) << "Trying to revert modified settings using invalid topology:\n"
                    << toJson(cached_state->m_modified.m_topology);
      return false;
    }

    if (!m_dd_api->setTopology(cached_state->m_modified.m_topology)) {
      DD_LOG(error) << "Failed to change topology to:\n"
                    << toJson(cached_state->m_modified.m_topology);
      return false;
    }

    DdGuardFn hdr_guard_fn;
    boost::scope::scope_exit<DdGuardFn &> hdr_guard { hdr_guard_fn, false };
    if (!cached_state->m_modified.m_original_hdr_states.empty()) {
      hdr_guard_fn = win_utils::hdrStateGuardFn(*m_dd_api, cached_state->m_modified.m_topology);
      hdr_guard.set_active(true);
      DD_LOG(info) << "Trying to change back the HDR states to:\n"
                   << toJson(cached_state->m_modified.m_original_hdr_states);
      if (!m_dd_api->setHdrStates(cached_state->m_modified.m_original_hdr_states)) {
        // Error already logged
        return false;
      }
    }

    DdGuardFn mode_guard_fn;
    boost::scope::scope_exit<DdGuardFn &> mode_guard { mode_guard_fn, false };
    if (!cached_state->m_modified.m_original_modes.empty()) {
      mode_guard_fn = win_utils::modeGuardFn(*m_dd_api, cached_state->m_modified.m_topology);
      mode_guard.set_active(true);
      DD_LOG(info) << "Trying to change back the display modes to:\n"
                   << toJson(cached_state->m_modified.m_original_modes);
      if (!m_dd_api->setDisplayModes(cached_state->m_modified.m_original_modes)) {
        // Error already logged
        return false;
      }
    }

    DdGuardFn primary_guard_fn;
    boost::scope::scope_exit<DdGuardFn &> primary_guard { primary_guard_fn, false };
    if (!cached_state->m_modified.m_original_primary_device.empty()) {
      primary_guard_fn = win_utils::primaryGuardFn(*m_dd_api, cached_state->m_modified.m_topology);
      primary_guard.set_active(true);
      DD_LOG(info) << "Trying to change back the original primary device to: " << toJson(cached_state->m_modified.m_original_primary_device);
      if (!m_dd_api->setAsPrimary(cached_state->m_modified.m_original_primary_device)) {
        // Error already logged
        return false;
      }
    }

    auto cleared_data { *cached_state };
    cleared_data.m_modified = { cleared_data.m_modified.m_topology };
    if (!m_persistence_state->persistState(cleared_data)) {
      DD_LOG(error) << "Failed to save reverted settings! Undoing changes to modified topology...";
      return false;
    }

    // Disable guards
    hdr_guard.set_active(false);
    mode_guard.set_active(false);
    primary_guard.set_active(false);
    return true;
  }
}  // namespace display_device
