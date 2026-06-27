/**
 * @file src/windows/factory.cpp
 * @brief Factory helpers for Windows platform interfaces.
 */
// class header include
#include "display_device/factory.h"

// system includes
#include <utility>

// local includes
#include "display_device/windows/display_power.h"
#include "display_device/windows/settings_manager.h"
#include "display_device/windows/win_api_layer.h"
#include "display_device/windows/win_display_device.h"

namespace display_device {
  std::unique_ptr<SettingsManagerInterface> makeSettingsManager(SettingsManagerFactoryConfig config) {
    auto api_layer {std::make_shared<WinApiLayer>()};
    return std::make_unique<SettingsManager>(
      std::make_shared<WinDisplayDevice>(api_layer),
      std::move(config.m_audio_context_api),
      std::make_unique<PersistentState>(std::move(config.m_settings_persistence_api), config.m_throw_on_persistence_load_error),
      WinWorkarounds {
        .m_hdr_blank_delay = config.m_hdr_blank_delay
      }
    );
  }

  std::unique_ptr<DisplayPowerInterface> makeDisplayPower() {
    return std::make_unique<WinDisplayPower>(std::make_shared<WinApiLayer>());
  }
}  // namespace display_device
