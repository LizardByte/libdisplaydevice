#pragma once

// system includes
#include <memory>

// local includes
#include "displaydevice/audiocontextinterface.h"
#include "displaydevice/settingsmanagerinterface.h"
#include "displaydevice/settingspersistenceinterface.h"
#include "displaydevice/windows/windisplaydeviceinterface.h"

namespace display_device {
  /**
   * @brief Default implementation for the SettingsManagerInterface.
   */
  class SettingsManager: public SettingsManagerInterface {
  public:
    /**
     * Default constructor for the class.
     * @param dd_api A pointer to the Windows Display Device interface. Will throw on nullptr!
     * @param settings_persistence_api [Optional] A pointer to the Settings Persistence interface.
     * @param audio_context_api [Optional] A pointer to the Audio Context interface.
     */
    explicit SettingsManager(
      std::shared_ptr<WinDisplayDeviceInterface> dd_api,
      std::shared_ptr<SettingsPersistenceInterface> settings_persistence_api,
      std::shared_ptr<AudioContextInterface> audio_context_api);

    /** For details @see SettingsManagerInterface::enumAvailableDevices */
    [[nodiscard]] EnumeratedDeviceList
    enumAvailableDevices() const override;

    /** For details @see SettingsManagerInterface::getDisplayName */
    [[nodiscard]] std::string
    getDisplayName(const std::string &device_id) const override;

  protected:
    std::shared_ptr<WinDisplayDeviceInterface> m_dd_api;
    std::shared_ptr<SettingsPersistenceInterface> m_settings_persistence_api;
    std::shared_ptr<AudioContextInterface> m_audio_context_api;
  };
}  // namespace display_device
