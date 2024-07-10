#pragma once

// system includes
#include <memory>

// local includes
#include "displaydevice/audiocontextinterface.h"
#include "displaydevice/settingsmanagerinterface.h"
#include "displaydevice/windows/windisplaydeviceinterface.h"
#include "persistentstate.h"

namespace display_device {
  /**
   * @brief Default implementation for the SettingsManagerInterface.
   */
  class SettingsManager: public SettingsManagerInterface {
  public:
    /**
     * Default constructor for the class.
     * @param dd_api A pointer to the Windows Display Device interface. Will throw on nullptr!
     * @param audio_context_api [Optional] A pointer to the Audio Context interface.
     * @param persistent_state A pointer to a class for managing persistence.
     */
    explicit
    SettingsManager(
      std::shared_ptr<WinDisplayDeviceInterface> dd_api,
      std::shared_ptr<AudioContextInterface> audio_context_api,
      std::unique_ptr<PersistentState> persistent_state);

    /** For details @see SettingsManagerInterface::enumAvailableDevices */
    [[nodiscard]] EnumeratedDeviceList
    enumAvailableDevices() const override;

    /** For details @see SettingsManagerInterface::getDisplayName */
    [[nodiscard]] std::string
    getDisplayName(const std::string &device_id) const override;

    /** For details @see SettingsManagerInterface::applySettings */
    [[nodiscard]] ApplyResult
    applySettings(const SingleDisplayConfiguration &config) override;

    /** For details @see SettingsManagerInterface::revertSettings */
    [[nodiscard]] bool
    revertSettings() override;

  protected:
    /**
     * @brief Preps the topology so that the further settings could be applied.
     * @param config Configuration to be used for preparing topology.
     * @param topology_before_changes The current topology before any changes.
     * @param release_context Specifies whether the audio context should be released at the very end IF everything else has succeeded.
     * @return A tuple of (new_state that is to be updated/persisted, device_to_configure, additional_devices_to_configure).
     */
    [[nodiscard]] std::optional<std::tuple<SingleDisplayConfigState, std::string, std::set<std::string>>>
    prepareTopology(const SingleDisplayConfiguration &config, const ActiveTopology &topology_before_changes, bool &release_context);

    /**
     * @brief Changes or restores the primary device based on the cached state, new state and configuration.
     * @param config Configuration to be used for preparing primary device.
     * @param device_to_configure The main device to be used for preparation.
     * @param guard_fn Reference to the guard function which will be set to restore original state (if needed) in case something else fails down the line.
     * @param new_state Reference to the new state which is to be updated accordingly.
     * @return True if no errors have occured, false otherwise.
     */
    [[nodiscard]] bool
    preparePrimaryDevice(const SingleDisplayConfiguration &config, const std::string &device_to_configure, DdGuardFn &guard_fn, SingleDisplayConfigState &new_state);

    /**
     * @brief Try to revert the modified settings.
     * @returns True on success, false otherwise.
     * @warning The method assumes that the caller will ensure restoring the topology
     *          in case of a failure!
     */
    [[nodiscard]] bool
    revertModifiedSettings();

    std::shared_ptr<WinDisplayDeviceInterface> m_dd_api;
    std::shared_ptr<AudioContextInterface> m_audio_context_api;
    std::unique_ptr<PersistentState> m_persistence_state;
  };
}  // namespace display_device
