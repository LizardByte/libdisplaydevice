/**
 * @file src/macos/settings_manager_apply.cpp
 * @brief Definitions for the methods for applying settings in MacSettingsManager.
 */
// class header include
#include "display_device/macos/settings_manager.h"

namespace display_device {
  MacSettingsManager::ApplyResult MacSettingsManager::applySettings(const SingleDisplayConfiguration &config) {
    if (!m_dd_api->isApiAccessAvailable()) {
      return ApplyResult::ApiTemporarilyUnavailable;
    }

    if (config.m_hdr_state) {
      return ApplyResult::HdrStatePrepFailed;
    }

    if (config.m_resolution || config.m_refresh_rate) {
      return ApplyResult::DisplayModePrepFailed;
    }

    return ApplyResult::DevicePrepFailed;
  }
}  // namespace display_device
