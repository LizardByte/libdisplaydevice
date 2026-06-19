/**
 * @file src/common/include/display_device/display_power_interface.h
 * @brief Declarations for display power management interfaces.
 */
#pragma once

// system includes
#include <chrono>
#include <memory>
#include <string>

namespace display_device {
  /**
   * @brief Scoped guard that keeps a display awake while it is alive.
   *
   * The guard owns the platform power assertion. Destroying the guard releases
   * that assertion. Some platforms attach the assertion to the current thread,
   * so callers should destroy the guard on the same thread that created it when
   * possible.
   */
  class DisplayPowerGuardInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~DisplayPowerGuardInterface() = default;
  };

  /**
   * @brief Cross-platform API for display wake and display-sleep prevention.
   *
   * This API prepares displays for capture. It does not change display
   * topology, primary display selection, resolution, refresh rate, HDR state,
   * or any persisted display settings.
   */
  class DisplayPowerInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~DisplayPowerInterface() = default;

    /**
     * @brief Ask the platform to wake a display before detection or capture.
     *
     * @param display_name Platform capture selector returned by SettingsManagerInterface::getDisplayName.
     * @param timeout Maximum time to wait for the display to become detectable.
     * @returns True if the platform wake operation succeeded and the display is considered detectable, false otherwise.
     */
    [[nodiscard]] virtual bool wakeDisplay(const std::string &display_name, std::chrono::milliseconds timeout) = 0;

    /**
     * @brief Keep displays awake until the returned guard is destroyed.
     *
     * @param reason Short human-readable reason for the platform power assertion.
     * @returns A guard that owns the assertion, or nullptr if the assertion could not be created.
     */
    [[nodiscard]] virtual std::unique_ptr<DisplayPowerGuardInterface> keepDisplayAwake(const std::string &reason) = 0;
  };
}  // namespace display_device
