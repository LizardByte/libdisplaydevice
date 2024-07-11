#pragma once

// local includes
#include "types.h"

namespace display_device {
  /**
   * @brief A class for applying and reverting display device settings.
   */
  class SettingsManagerInterface {
  public:
    /**
     * @brief Outcome values when trying to apply settings.
     */
    enum class ApplyResult {
      Ok,
      ApiTemporarilyUnavailable,
      DevicePrepFailed,
      PrimaryDevicePrepFailed,
      DisplayModePrepFailed,
      HdrStatePrepFailed,
      PersistenceSaveFailed,
    };

    /**
     * @brief Default virtual destructor.
     */
    virtual ~SettingsManagerInterface() = default;

    /**
     * @brief Enumerate the available (active and inactive) devices.
     * @returns A list of available devices.
     *          Empty list can also be returned if an error has occurred.
     *
     * EXAMPLES:
     * ```cpp
     * const SettingsManagerInterface* iface = getIface(...);
     * const auto devices { iface->enumAvailableDevices() };
     * ```
     */
    [[nodiscard]] virtual EnumeratedDeviceList
    enumAvailableDevices() const = 0;

    /**
     * @brief Get display name associated with the device.
     * @param device_id A device to get display name for.
     * @returns A display name for the device, or an empty string if the device is inactive or not found.
     *          Empty string can also be returned if an error has occurred.
     *
     * EXAMPLES:
     * ```cpp
     * const std::string device_id { "MY_DEVICE_ID" };
     * const SettingsManagerInterface* iface = getIface(...);
     * const std::string display_name = iface->getDisplayName(device_id);
     * ```
     */
    [[nodiscard]] virtual std::string
    getDisplayName(const std::string &device_id) const = 0;

    /**
     * @brief Apply the provided configuration to the system.
     * @param config A desired configuration for the display device.
     * @returns The apply result.
     *
     * EXAMPLES:
     * ```cpp
     * const SingleDisplayConfiguration config;
     *
     * SettingsManagerInterface* iface = getIface(...);
     * const auto result = iface->applySettings(config);
     * ```
     */
    [[nodiscard]] virtual ApplyResult
    applySettings(const SingleDisplayConfiguration &config) = 0;

    /**
     * @brief Revert the applied configuration and restore the previous settings.
     * @returns True if settings were reverted or there was nothing to revert, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * SettingsManagerInterface* iface = getIface(...);
     * const auto result = iface->revertSettings();
     * ```
     */
    [[nodiscard]] virtual bool
    revertSettings() = 0;

    /**
     * @brief Reset the persistence in case the settings cannot be reverted.
     *
     * In case the settings cannot be reverted, because the display is turned or some other reason,
     * this allows to "accept" the current state and start from scratch.
     *
     * EXAMPLES:
     * ```cpp
     * SettingsManagerInterface* iface = getIface(...);
     * const auto result = iface->applySettings(config);
     * if (result == ApplyResult::Ok) {
     *   // Wait for some time
     *   if (!iface->revertSettings()) {
     *     // Wait for user input
     *     const bool user_wants_reset { true };
     *     if (user_wants_reset) {
     *       iface->resetPersistence();
     *     }
     *   }
     * }
     * ```
     */
    // virtual void
    // resetPersistence() = 0;
  };
}  // namespace display_device
