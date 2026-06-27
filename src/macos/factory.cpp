/**
 * @file src/macos/factory.cpp
 * @brief Factory helpers for macOS platform interfaces.
 */
// class header include
#include "display_device/factory.h"

// system includes
#include <utility>

// local includes
#include "display_device/macos/display_power.h"
#include "display_device/macos/mac_api_layer.h"
#include "display_device/macos/mac_display_device.h"
#include "display_device/macos/settings_manager.h"

namespace display_device {
  std::unique_ptr<SettingsManagerInterface> makeSettingsManager(SettingsManagerFactoryConfig config) {
    auto api_layer {std::make_shared<MacApiLayer>()};
    return std::make_unique<MacSettingsManager>(
      std::make_shared<MacDisplayDevice>(api_layer),
      std::move(config.m_audio_context_api),
      std::make_unique<MacPersistentState>(std::move(config.m_settings_persistence_api), config.m_throw_on_persistence_load_error),
      MacWorkarounds {}
    );
  }

  std::unique_ptr<DisplayPowerInterface> makeDisplayPower() {
    return std::make_unique<MacDisplayPower>(std::make_shared<MacApiLayer>());
  }
}  // namespace display_device
