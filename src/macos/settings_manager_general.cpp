/**
 * @file src/macos/settings_manager_general.cpp
 * @brief Definitions for the leftover general methods in MacSettingsManager.
 */
// class header include
#include "display_device/macos/settings_manager.h"

// system includes
#include <stdexcept>

// local includes
#include "display_device/noop_audio_context.h"

namespace display_device {
  MacSettingsManager::MacSettingsManager(
    std::shared_ptr<MacDisplayDeviceInterface> dd_api,
    std::shared_ptr<AudioContextInterface> audio_context_api,
    std::unique_ptr<MacPersistentState> persistent_state,
    MacWorkarounds workarounds
  ):
      m_dd_api {std::move(dd_api)},
      m_audio_context_api {std::move(audio_context_api)},
      m_persistence_state {std::move(persistent_state)},
      m_workarounds {std::move(workarounds)} {
    if (!m_dd_api) {
      throw std::invalid_argument {"Nullptr provided for MacDisplayDeviceInterface in MacSettingsManager!"};
    }

    if (!m_audio_context_api) {
      m_audio_context_api = std::make_shared<NoopAudioContext>();
    }

    if (!m_persistence_state) {
      throw std::invalid_argument {"Nullptr provided for MacPersistentState in MacSettingsManager!"};
    }
  }

  EnumeratedDeviceList MacSettingsManager::enumAvailableDevices() const {
    return m_dd_api->enumAvailableDevices();
  }

  std::string MacSettingsManager::getDisplayName(const std::string &device_id) const {
    return m_dd_api->getDisplayName(device_id);
  }

  const std::shared_ptr<AudioContextInterface> &MacSettingsManager::getAudioContextApi() const {
    return m_audio_context_api;
  }

  bool MacSettingsManager::resetPersistence() {
    if (const auto &cached_state {m_persistence_state->getState()}; !cached_state) {
      return true;
    }

    if (!m_persistence_state->persistState(std::nullopt)) {
      return false;
    }

    if (m_audio_context_api->isCaptured()) {
      m_audio_context_api->release();
    }
    return true;
  }
}  // namespace display_device
