// class header include
#include "displaydevice/windows/settings_manager.h"

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/noop_audio_context.h"

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

  bool
  SettingsManager::resetPersistence() {
    // Trying to revert one more time in case we succeed.
    if (revertSettings()) {
      return true;
    }

    DD_LOG(info) << "Trying to reset persistent display device settings.";
    if (!m_persistence_state->persistState(std::nullopt)) {
      DD_LOG(error) << "Failed to clear persistence!";
      return false;
    }

    if (m_audio_context_api->isCaptured()) {
      m_audio_context_api->release();
    }
    return true;
  }
}  // namespace display_device
