/**
 * @file src/windows/include/display_device/windows/win_display_device.h
 * @brief Declarations for the WinDisplayDevice.
 */
#pragma once

// system includes
#include <memory>

// local includes
#include "win_api_layer_interface.h"
#include "win_display_device_interface.h"

namespace display_device {
  /**
   * @brief Default implementation for the WinDisplayDeviceInterface.
   */
  class WinDisplayDevice: public WinDisplayDeviceInterface {
  public:
    /**
     * Default constructor for the class.
     * @param w_api A pointer to the Windows API layer. Will throw on nullptr!
     */
    explicit WinDisplayDevice(std::shared_ptr<WinApiLayerInterface> w_api);

    /**
     * @copydoc WinDisplayDeviceInterface::isApiAccessAvailable
     */
    [[nodiscard]] bool isApiAccessAvailable() const override;

    /**
     * @copydoc WinDisplayDeviceInterface::enumAvailableDevices
     */
    [[nodiscard]] EnumeratedDeviceList enumAvailableDevices() const override;

    /**
     * @copydoc WinDisplayDeviceInterface::getDisplayName
     */
    [[nodiscard]] std::string getDisplayName(const std::string &device_id) const override;

    /**
     * @copydoc WinDisplayDeviceInterface::getCurrentTopology
     */
    [[nodiscard]] ActiveTopology getCurrentTopology() const override;

    /**
     * @copydoc WinDisplayDeviceInterface::isTopologyValid
     */
    [[nodiscard]] bool isTopologyValid(const ActiveTopology &topology) const override;

    /**
     * @copydoc WinDisplayDeviceInterface::isTopologyTheSame
     */
    [[nodiscard]] bool isTopologyTheSame(const ActiveTopology &lhs, const ActiveTopology &rhs) const override;

    /**
     * @copydoc WinDisplayDeviceInterface::setTopology
     */
    [[nodiscard]] bool setTopology(const ActiveTopology &new_topology) override;

    /**
     * @copydoc WinDisplayDeviceInterface::getCurrentDisplayModes
     */
    [[nodiscard]] DeviceDisplayModeMap getCurrentDisplayModes(const StringSet &device_ids) const override;

    /**
     * @copydoc WinDisplayDeviceInterface::setDisplayModes
     */
    [[nodiscard]] bool setDisplayModes(const DeviceDisplayModeMap &modes) override;

    /**
     * @copydoc WinDisplayDeviceInterface::isPrimary
     */
    [[nodiscard]] bool isPrimary(const std::string &device_id) const override;

    /**
     * @copydoc WinDisplayDeviceInterface::setAsPrimary
     */
    [[nodiscard]] bool setAsPrimary(const std::string &device_id) override;

    /**
     * @copydoc WinDisplayDeviceInterface::getCurrentHdrStates
     */
    [[nodiscard]] HdrStateMap getCurrentHdrStates(const StringSet &device_ids) const override;

    /**
     * @copydoc WinDisplayDeviceInterface::setHdrStates
     */
    [[nodiscard]] bool setHdrStates(const HdrStateMap &states) override;

  private:
    std::shared_ptr<WinApiLayerInterface> m_w_api;
  };
}  // namespace display_device
