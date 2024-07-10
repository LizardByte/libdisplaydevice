#pragma once

namespace display_device {
  /**
   * @brief A class for capturing associated audio context (settings, info or whatever).
   *
   * Some of the display devices have audio devices associated with them.
   * Turning off and on the devices will not necessarily restore them as the default
   * audio devices for the system.
   */
  class AudioContextInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~AudioContextInterface() = default;

    /**
     * @brief Capture audio context for currently active devices.
     * @returns True if the contexts could be captured, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * AudioContextInterface* iface = getIface(...);
     * const auto result { iface->capture() };
     * ```
     */
    [[nodiscard]] virtual bool
    capture() = 0;

    /**
     * @brief Check if the context is already captured.
     * @returns True if the context is captured, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * AudioContextInterface* iface = getIface(...);
     * const auto result { iface->isCaptured() };
     * ```
     */
    [[nodiscard]] virtual bool
    isCaptured() const = 0;

    /**
     * @brief Release captured audio context for the devices (if any).
     *
     * EXAMPLES:
     * ```cpp
     * AudioContextInterface* iface = getIface(...);
     * const auto result { iface->release() };
     * ```
     */
    virtual void
    release() = 0;
  };
}  // namespace display_device
