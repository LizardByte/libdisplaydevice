// class header include
#include "displaydevice/windows/persistentstate.h"

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/noopsettingspersistence.h"
#include "displaydevice/windows/json.h"

namespace display_device {
  PersistentState::PersistentState(std::shared_ptr<SettingsPersistenceInterface> settings_persistence_api, const std::optional<SingleDisplayConfigState> &fallback_state):
      m_settings_persistence_api { std::move(settings_persistence_api) } {
    if (!m_settings_persistence_api) {
      m_settings_persistence_api = std::make_shared<NoopSettingsPersistence>();
    }

    std::string error_message;
    if (const auto persistent_settings { m_settings_persistence_api->load() }) {
      if (!persistent_settings->empty()) {
        m_cached_state = SingleDisplayConfigState {};
        if (!fromJson({ std::begin(*persistent_settings), std::end(*persistent_settings) }, *m_cached_state, &error_message)) {
          error_message = "Failed to parse persistent settings! Error:\n" + error_message;
        }
      }
    }
    else {
      error_message = "Failed to load persistent settings!";
    }

    if (!error_message.empty()) {
      if (!fallback_state) {
        throw std::runtime_error { error_message };
      }

      m_cached_state = *fallback_state;
    }
  }

  bool
  PersistentState::persistState(const std::optional<SingleDisplayConfigState> &state) {
    if (m_cached_state == state) {
      return true;
    }

    if (!state) {
      if (!m_settings_persistence_api->clear()) {
        return false;
      }

      m_cached_state = std::nullopt;
      return true;
    }

    bool success { false };
    const auto json_string { toJson(*state, 2, &success) };
    if (!success) {
      DD_LOG(error) << "Failed to serialize new persistent state! Error:\n"
                    << json_string;
      return false;
    }

    if (!m_settings_persistence_api->store({ std::begin(json_string), std::end(json_string) })) {
      return false;
    }

    m_cached_state = *state;
    return true;
  }

  const std::optional<SingleDisplayConfigState> &
  PersistentState::getState() const {
    return m_cached_state;
  }
}  // namespace display_device
