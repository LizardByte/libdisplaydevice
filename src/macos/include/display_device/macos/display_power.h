/**
 * @file src/macos/include/display_device/macos/display_power.h
 * @brief Declarations for macOS display power management.
 */
#pragma once

// system includes
#include <memory>

// local includes
#include "display_device/display_power_interface.h"
#include "mac_api_layer_interface.h"

namespace display_device {
  /**
   * @brief macOS implementation of DisplayPowerInterface.
   */
  class MacDisplayPower: public DisplayPowerInterface {
  public:
    /**
     * @brief Default constructor for the class.
     * @param m_api A pointer to the macOS API layer. Will throw on nullptr.
     */
    explicit MacDisplayPower(std::shared_ptr<MacApiLayerInterface> m_api);

    /**
     * @copydoc DisplayPowerInterface::wakeDisplay
     */
    [[nodiscard]] bool wakeDisplay(const std::string &display_name, std::chrono::milliseconds timeout) override;

    /**
     * @copydoc DisplayPowerInterface::keepDisplayAwake
     */
    [[nodiscard]] std::unique_ptr<DisplayPowerGuardInterface> keepDisplayAwake(const std::string &reason) override;

  private:
    /**
     * @brief Check if the requested display is already active.
     * @param display_name Platform capture selector to check.
     * @returns True if the requested display is active, false otherwise.
     */
    [[nodiscard]] bool hasRequiredActiveDisplay(const std::string &display_name) const;

    std::shared_ptr<MacApiLayerInterface> m_m_api;  ///< macOS API layer.
  };
}  // namespace display_device
