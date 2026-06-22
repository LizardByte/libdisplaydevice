/**
 * @file src/macos/include/display_device/macos/persistent_state.h
 * @brief Declarations for the MacPersistentState.
 */
#pragma once

// system includes
#include <memory>

// local includes
#include "display_device/settings_persistence_interface.h"
#include "types.h"

namespace display_device {
  /**
   * @brief A wrapper around SettingsPersistenceInterface and cached macOS state.
   */
  class MacPersistentState {
  public:
    /**
     * @brief Default constructor for the class.
     * @param settings_persistence_api Optional settings persistence interface.
     * @param throw_on_load_error Specify whether to throw in constructor if settings fail to load.
     */
    explicit MacPersistentState(std::shared_ptr<SettingsPersistenceInterface> settings_persistence_api, bool throw_on_load_error = false);

    /**
     * @brief Store the new state via the interface and cache it.
     * @param state New state to set.
     * @return True if the state was successfully updated, false otherwise.
     */
    [[nodiscard]] bool persistState(const std::optional<MacSingleDisplayConfigState> &state);

    /**
     * @brief Get cached state.
     * @return Cached state.
     */
    [[nodiscard]] const std::optional<MacSingleDisplayConfigState> &getState() const;

    /**
     * @brief Get the settings persistence API.
     * @returns Settings persistence API.
     */
    [[nodiscard]] const std::shared_ptr<SettingsPersistenceInterface> &getSettingsPersistenceApi() const;

  private:
    std::shared_ptr<SettingsPersistenceInterface> m_settings_persistence_api;
    std::optional<MacSingleDisplayConfigState> m_cached_state;
  };
}  // namespace display_device
