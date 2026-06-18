/**
 * @file src/macos/settings_manager_revert.cpp
 * @brief Definitions for the methods for reverting settings in MacSettingsManager.
 */
// class header include
#include "display_device/macos/settings_manager.h"

namespace display_device {
  MacSettingsManager::RevertResult MacSettingsManager::revertSettings() {
    const auto &cached_state {m_persistence_state->getState()};
    if (!cached_state) {
      return RevertResult::Ok;
    }

    if (!m_dd_api->isApiAccessAvailable()) {
      return RevertResult::ApiTemporarilyUnavailable;
    }

    return RevertResult::SwitchingTopologyFailed;
  }
}  // namespace display_device
