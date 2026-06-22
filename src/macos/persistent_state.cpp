/**
 * @file src/macos/persistent_state.cpp
 * @brief Definitions for the MacPersistentState.
 */
// class header include
#include "display_device/macos/persistent_state.h"

// system includes
#include <stdexcept>

// local includes
#include "display_device/detail/persistent_state_utils.h"
#include "display_device/logging.h"
#include "display_device/macos/json.h"
#include "display_device/noop_settings_persistence.h"

namespace display_device {
  namespace {
    /**
     * @brief Exception thrown when macOS persistent state loading fails.
     */
    class MacPersistentStateLoadException final: public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
    };
  }  // namespace

  MacPersistentState::MacPersistentState(std::shared_ptr<SettingsPersistenceInterface> settings_persistence_api, const bool throw_on_load_error):
      m_settings_persistence_api {std::move(settings_persistence_api)} {
    if (!m_settings_persistence_api) {
      m_settings_persistence_api = std::make_shared<NoopSettingsPersistence>();
    }

    std::string error_message;
    if (const auto persistent_settings {m_settings_persistence_api->load()}) {
      if (!persistent_settings->empty()) {
        m_cached_state = MacSingleDisplayConfigState {};
        if (!fromJson({std::begin(*persistent_settings), std::end(*persistent_settings)}, *m_cached_state, &error_message)) {
          error_message = "Failed to parse macOS persistent settings! Error:\n" + error_message;
        }
      }
    } else {
      error_message = "Failed to load macOS persistent settings!";
    }

    if (!error_message.empty()) {
      if (throw_on_load_error) {
        throw MacPersistentStateLoadException {error_message};
      }

      DD_LOG(error) << error_message;
      m_cached_state = std::nullopt;
    }
  }

  bool MacPersistentState::persistState(const std::optional<MacSingleDisplayConfigState> &state) {
    return detail::persistState(
      *m_settings_persistence_api,
      m_cached_state,
      state,
      [](const MacSingleDisplayConfigState &state_to_serialize, bool &success) {
        return toJson(state_to_serialize, 2, &success);
      },
      "Failed to serialize new macOS persistent state! Error:"
    );
  }

  const std::optional<MacSingleDisplayConfigState> &MacPersistentState::getState() const {
    return m_cached_state;
  }

  const std::shared_ptr<SettingsPersistenceInterface> &MacPersistentState::getSettingsPersistenceApi() const {
    return m_settings_persistence_api;
  }
}  // namespace display_device
