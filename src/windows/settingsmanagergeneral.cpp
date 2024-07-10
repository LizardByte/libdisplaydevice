// class header include
#include "displaydevice/windows/settingsmanager.h"

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/noopaudiocontext.h"

namespace display_device {
  SettingsManager::SettingsManager(
    std::shared_ptr<WinDisplayDeviceInterface> dd_api,
    std::shared_ptr<AudioContextInterface> audio_context_api,
    std::unique_ptr<PersistentState> persistent_state):
      m_dd_api { std::move(dd_api) },
      m_audio_context_api { std::move(audio_context_api) },
      m_persistence_state { std::move(persistent_state) } {
    if (!m_dd_api) {
      throw std::logic_error { "Nullptr provided for WinDisplayDeviceInterface in SettingsManager!" };
    }

    if (!m_audio_context_api) {
      m_audio_context_api = std::make_shared<NoopAudioContext>();
    }

    if (!m_persistence_state) {
      throw std::logic_error { "Nullptr provided for PersistentState in SettingsManager!" };
    }
  }

  EnumeratedDeviceList
  SettingsManager::enumAvailableDevices() const {
    return m_dd_api->enumAvailableDevices();
  }

  std::string
  SettingsManager::getDisplayName(const std::string &device_id) const {
    return m_dd_api->getDisplayName(device_id);
  }
}  // namespace display_device
