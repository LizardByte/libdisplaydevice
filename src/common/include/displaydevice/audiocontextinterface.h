#pragma once

// system includes
#include <string>
#include <vector>

namespace display_device {
  /**
   * @brief A class for capturing associated audio context (settings, info or whatever).
   *
   * Some of the display devices have audio devices associated with them.
   * Turning off and on the devices will not necessarily restore them as the default
   * audio devices for the system.
   *
   * While this library does not preserve audio contexts, it does provide this interface
   * for notifying which devices are about to be disabled (their audio context should
   * probably be "captured") and which ones are to be re-enabled (their audio context should
   * probably be "released").
   */
  class AudioContextInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~AudioContextInterface() = default;

    /**
     * @brief Capture audio context for the devices that are about to be disabled.
     * @param devices_to_be_disabled Devices that might be disabled soon.
     * @returns True if the contexts could be captured, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * const std::string device_id { "MY_DEVICE_ID" };
     * AudioContextInterface* iface = getIface(...);
     * const auto result { iface->capture({ device_id }) };
     * ```
     */
    [[nodiscard]] virtual bool
    capture(const std::vector<std::string> &devices_to_be_disabled) = 0;

    /**
     * @brief Release audio context for the devices that are about to be re-enabled.
     * @param devices_to_be_reenabled Devices that were captured before and are about to be re-enabled.
     *
     * EXAMPLES:
     * ```cpp
     * const std::string device_id { "MY_DEVICE_ID" };
     * AudioContextInterface* iface = getIface(...);
     * const auto result { iface->release({ device_id }) };
     * ```
     */
    virtual void
    release(const std::vector<std::string> &devices_to_be_reenabled) = 0;
  };
}  // namespace display_device
