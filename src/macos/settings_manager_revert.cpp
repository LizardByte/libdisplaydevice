/**
 * @file src/macos/settings_manager_revert.cpp
 * @brief Definitions for the methods for reverting settings in MacSettingsManager.
 */
// class header include
#include "display_device/macos/settings_manager.h"

// local includes
#include "display_device/logging.h"
#include "display_device/macos/json.h"
#include "display_device/macos/settings_utils.h"

namespace display_device {
  MacSettingsManager::RevertResult MacSettingsManager::revertSettings() {
    const auto &cached_state {m_persistence_state->getState()};
    if (!cached_state) {
      return RevertResult::Ok;
    }

    if (!m_dd_api->isApiAccessAvailable()) {
      return RevertResult::ApiTemporarilyUnavailable;
    }

    const auto current_topology {m_dd_api->getCurrentTopology()};
    if (!m_dd_api->isTopologyValid(current_topology)) {
      DD_LOG(error) << "Retrieved current macOS topology is invalid:\n"
                    << toJson(current_topology);
      return RevertResult::TopologyIsInvalid;
    }

    if (!m_dd_api->isTopologyValid(cached_state->m_modified.m_topology)) {
      DD_LOG(error) << "Trying to revert macOS modes using invalid modified topology:\n"
                    << toJson(cached_state->m_modified.m_topology);
      return RevertResult::TopologyIsInvalid;
    }

    if (!m_dd_api->isTopologyTheSame(current_topology, cached_state->m_modified.m_topology)) {
      DD_LOG(error) << "Cannot revert macOS display modes because topology changes are not supported in phase 2.";
      return RevertResult::SwitchingTopologyFailed;
    }

    bool rollback_modes_on_failure {false};
    MacDeviceDisplayModeMap modes_to_restore;

    if (!cached_state->m_modified.m_original_hdr_states.empty()) {
      DD_LOG(error) << "Cannot revert macOS HDR state because HDR mutations are unsupported.";
      return RevertResult::RevertingHdrStatesFailed;
    }

    if (!cached_state->m_modified.m_original_primary_device.empty()) {
      DD_LOG(error) << "Cannot revert macOS primary display because primary mutations are unsupported in phase 2.";
      return RevertResult::RevertingPrimaryDeviceFailed;
    }

    if (!cached_state->m_modified.m_original_modes.empty()) {
      const auto mode_devices {mac_utils::flattenTopology(cached_state->m_modified.m_topology)};
      const auto current_modes {m_dd_api->getCurrentDisplayModes(mode_devices)};
      if (current_modes.empty()) {
        DD_LOG(error) << "Failed to get current macOS display modes for revert!";
        return RevertResult::RevertingDisplayModesFailed;
      }

      if (current_modes != cached_state->m_modified.m_original_modes) {
        DD_LOG(info) << "Trying to change back macOS display modes to:\n"
                     << toJson(cached_state->m_modified.m_original_modes);
        if (!m_dd_api->setDisplayModes(cached_state->m_modified.m_original_modes)) {
          return RevertResult::RevertingDisplayModesFailed;
        }

        rollback_modes_on_failure = true;
        modes_to_restore = current_modes;
      }
    }

    if (!m_persistence_state->persistState(std::nullopt)) {
      DD_LOG(error) << "Failed to clear reverted macOS display settings! Undoing mode changes...";
      if (rollback_modes_on_failure) {
        static_cast<void>(m_dd_api->setDisplayModes(modes_to_restore));
      }
      return RevertResult::PersistenceSaveFailed;
    }

    if (m_audio_context_api->isCaptured()) {
      m_audio_context_api->release();
    }

    return RevertResult::Ok;
  }
}  // namespace display_device
