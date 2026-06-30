/**
 * @file src/common/include/display_device/factory.h
 * @brief Factory helpers for platform display device interfaces.
 */
#pragma once

// system includes
#include <chrono>
#include <memory>
#include <optional>

// local includes
#include "audio_context_interface.h"
#include "display_power_interface.h"
#include "settings_manager_interface.h"
#include "settings_persistence_interface.h"

namespace display_device {
  /**
   * @brief Dependencies and platform-neutral options for creating a settings manager.
   */
  struct SettingsManagerFactoryConfig {
    std::shared_ptr<AudioContextInterface> m_audio_context_api {};  ///< Optional audio context interface.
    std::shared_ptr<SettingsPersistenceInterface> m_settings_persistence_api {};  ///< Optional settings persistence interface.
    bool m_throw_on_persistence_load_error {};  ///< Throw when persisted settings cannot be loaded or parsed.
    std::optional<std::chrono::milliseconds> m_hdr_blank_delay {};  ///< Optional HDR blanking workaround delay on supported platforms.
  };

  /**
   * @brief Create the default settings manager for the current platform.
   * @param config Dependencies and platform-neutral options.
   * @returns A settings manager, or nullptr when the platform is unsupported.
   */
  [[nodiscard]] std::unique_ptr<SettingsManagerInterface> makeSettingsManager(const SettingsManagerFactoryConfig &config = {});

  /**
   * @brief Create the default display power manager for the current platform.
   * @returns A display power manager, or nullptr when the platform is unsupported.
   */
  [[nodiscard]] std::unique_ptr<DisplayPowerInterface> makeDisplayPower();
}  // namespace display_device
