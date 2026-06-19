/**
 * @file src/macos/include/display_device/macos/settings_manager.h
 * @brief Declarations for the MacSettingsManager.
 */
#pragma once

// system includes
#include <memory>

// local includes
#include "display_device/audio_context_interface.h"
#include "display_device/settings_manager_interface.h"
#include "mac_display_device_interface.h"
#include "persistent_state.h"

namespace display_device {
  /**
   * @brief Default macOS implementation for the SettingsManagerInterface.
   *
   * macOS v1a supports `SingleDisplayConfiguration::DevicePreparation::VerifyOnly`
   * with active-display resolution and refresh-rate changes. HDR writes and topology
   * preparation modes such as `EnsureActive`, `EnsurePrimary`, and
   * `EnsureOnlyDisplay` fail explicitly.
   */
  class MacSettingsManager: public SettingsManagerInterface {
  public:
    /**
     * @brief Default constructor for the class.
     * @param dd_api A pointer to the macOS Display Device interface. Will throw on nullptr.
     * @param audio_context_api Optional Audio Context interface.
     * @param persistent_state A pointer to a class for managing persistence.
     * @param workarounds Workaround settings for the APIs.
     */
    explicit MacSettingsManager(
      std::shared_ptr<MacDisplayDeviceInterface> dd_api,
      std::shared_ptr<AudioContextInterface> audio_context_api,
      std::unique_ptr<MacPersistentState> persistent_state,
      MacWorkarounds workarounds
    );

    /**
     * @copydoc SettingsManagerInterface::enumAvailableDevices
     */
    [[nodiscard]] EnumeratedDeviceList enumAvailableDevices() const override;

    /**
     * @copydoc SettingsManagerInterface::getDisplayName
     */
    [[nodiscard]] std::string getDisplayName(const std::string &device_id) const override;

    /**
     * @copydoc SettingsManagerInterface::applySettings
     */
    [[nodiscard]] ApplyResult applySettings(const SingleDisplayConfiguration &config) override;

    /**
     * @copydoc SettingsManagerInterface::revertSettings
     */
    [[nodiscard]] RevertResult revertSettings() override;

    /**
     * @copydoc SettingsManagerInterface::resetPersistence
     */
    [[nodiscard]] bool resetPersistence() override;

    /**
     * @brief Get the audio context API.
     * @returns Audio context API.
     */
    [[nodiscard]] const std::shared_ptr<AudioContextInterface> &getAudioContextApi() const;

  private:
    std::shared_ptr<MacDisplayDeviceInterface> m_dd_api;
    std::shared_ptr<AudioContextInterface> m_audio_context_api;
    std::unique_ptr<MacPersistentState> m_persistence_state;
    MacWorkarounds m_workarounds;
  };
}  // namespace display_device
