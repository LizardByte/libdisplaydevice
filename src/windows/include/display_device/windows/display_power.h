/**
 * @file src/windows/include/display_device/windows/display_power.h
 * @brief Declarations for Windows display power management.
 */
#pragma once

// system includes
#include <memory>

// local includes
#include "display_device/display_power_interface.h"
#include "win_api_layer_interface.h"

namespace display_device {
  /**
   * @brief Windows implementation of DisplayPowerInterface.
   */
  class WinDisplayPower: public DisplayPowerInterface {
  public:
    /**
     * @brief Default constructor for the class.
     * @param w_api A pointer to the Windows API layer. Will throw on nullptr.
     */
    explicit WinDisplayPower(std::shared_ptr<WinApiLayerInterface> w_api);

    /**
     * @copydoc DisplayPowerInterface::wakeDisplay
     */
    [[nodiscard]] bool wakeDisplay(const std::string &display_name, std::chrono::milliseconds timeout) override;

    /**
     * @copydoc DisplayPowerInterface::keepDisplayAwake
     */
    [[nodiscard]] std::unique_ptr<DisplayPowerGuardInterface> keepDisplayAwake(const std::string &reason) override;

  private:
    std::shared_ptr<WinApiLayerInterface> m_w_api;  ///< Windows API layer.
  };
}  // namespace display_device
