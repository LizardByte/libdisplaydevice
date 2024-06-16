// class header include
#include "displaydevice/windows/settingsmanager.h"

// local includes
#include "displaydevice/noopaudiocontext.h"
#include "displaydevice/noopsettingspersistence.h"

namespace display_device {
  SettingsManager::SettingsManager(
    std::shared_ptr<WinDisplayDeviceInterface> dd_api,
    std::shared_ptr<SettingsPersistenceInterface> settings_persistence_api,
    std::shared_ptr<AudioContextInterface> audio_context_api):
      m_dd_api { std::move(dd_api) }, m_settings_persistence_api { std::move(settings_persistence_api) }, m_audio_context_api { std::move(audio_context_api) } {
    if (!m_dd_api) {
      throw std::logic_error { "Nullptr provided for WinDisplayDeviceInterface in SettingsManager!" };
    }

    if (!m_settings_persistence_api) {
      m_settings_persistence_api = std::make_shared<NoopSettingsPersistence>();
    }

    if (!m_audio_context_api) {
      m_audio_context_api = std::make_shared<NoopAudioContext>();
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
