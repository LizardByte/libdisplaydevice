#pragma once

// system includes
#include <memory>

// local includes
#include "displaydevice/settings_persistence_interface.h"
#include "displaydevice/windows/win_display_device_interface.h"

namespace display_device {
  /**
   * @brief A simple wrapper around the SettingsPersistenceInterface and cached local state to keep them in sync.
   */
  class PersistentState {
  public:
    /**
     * Default constructor for the class.
     * @param settings_persistence_api [Optional] A pointer to the Settings Persistence interface.
     * @param fallback_state Fallback state to be used if the persistence interface fails to load data (or it is invalid).
     *                       If no fallback is provided in such case, the constructor will throw.
     */
    explicit PersistentState(std::shared_ptr<SettingsPersistenceInterface> settings_persistence_api, const std::optional<SingleDisplayConfigState> &fallback_state = SingleDisplayConfigState {});

    /**
     * @brief Store the new state via the interface and cache it.
     * @param state New state to be set.
     * @return True if the state was succesfully updated, false otherwise.
     */
    [[nodiscard]] bool
    persistState(const std::optional<SingleDisplayConfigState> &state);

    /**
     * @brief Get cached state.
     * @return Cached state
     */
    [[nodiscard]] const std::optional<SingleDisplayConfigState> &
    getState() const;

  protected:
    std::shared_ptr<SettingsPersistenceInterface> m_settings_persistence_api;

  private:
    std::optional<SingleDisplayConfigState> m_cached_state;
  };
}  // namespace display_device
