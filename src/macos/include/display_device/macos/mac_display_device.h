/**
 * @file src/macos/include/display_device/macos/mac_display_device.h
 * @brief Declarations for the MacDisplayDevice.
 */
#pragma once

// system includes
#include <memory>
#include <string_view>

// local includes
#include "mac_api_layer_interface.h"
#include "mac_display_device_interface.h"

namespace display_device {
  /**
   * @brief Default implementation for the MacDisplayDeviceInterface.
   */
  class MacDisplayDevice: public MacDisplayDeviceInterface {
  public:
    /**
     * @brief Default constructor for the class.
     * @param m_api A pointer to the macOS API layer. Will throw on nullptr.
     */
    explicit MacDisplayDevice(std::shared_ptr<MacApiLayerInterface> m_api);

    /**
     * @copydoc MacDisplayDeviceInterface::isApiAccessAvailable
     */
    [[nodiscard]] bool isApiAccessAvailable() const override;

    /**
     * @copydoc MacDisplayDeviceInterface::enumAvailableDevices
     */
    [[nodiscard]] EnumeratedDeviceList enumAvailableDevices() const override;

    /**
     * @copydoc MacDisplayDeviceInterface::getDisplayName
     */
    [[nodiscard]] std::string getDisplayName(const std::string &device_id) const override;

    /**
     * @copydoc MacDisplayDeviceInterface::getCurrentTopology
     */
    [[nodiscard]] MacActiveTopology getCurrentTopology() const override;

    /**
     * @copydoc MacDisplayDeviceInterface::isTopologyValid
     */
    [[nodiscard]] bool isTopologyValid(const MacActiveTopology &topology) const override;

    /**
     * @copydoc MacDisplayDeviceInterface::isTopologyTheSame
     */
    [[nodiscard]] bool isTopologyTheSame(const MacActiveTopology &lhs, const MacActiveTopology &rhs) const override;

    /**
     * @copydoc MacDisplayDeviceInterface::setTopology
     */
    [[nodiscard]] bool setTopology(const MacActiveTopology &new_topology) override;

    /**
     * @copydoc MacDisplayDeviceInterface::getCurrentDisplayModes
     */
    [[nodiscard]] MacDeviceDisplayModeMap getCurrentDisplayModes(const StringSet &device_ids) const override;

    /**
     * @copydoc MacDisplayDeviceInterface::setDisplayModes
     */
    [[nodiscard]] bool setDisplayModes(const MacDeviceDisplayModeMap &modes) override;

    /**
     * @copydoc MacDisplayDeviceInterface::isPrimary
     */
    [[nodiscard]] bool isPrimary(const std::string &device_id) const override;

    /**
     * @copydoc MacDisplayDeviceInterface::setAsPrimary
     */
    [[nodiscard]] bool setAsPrimary(const std::string &device_id) override;

    /**
     * @copydoc MacDisplayDeviceInterface::getCurrentHdrStates
     */
    [[nodiscard]] MacHdrStateMap getCurrentHdrStates(const StringSet &device_ids) const override;

    /**
     * @copydoc MacDisplayDeviceInterface::setHdrStates
     */
    [[nodiscard]] bool setHdrStates(const MacHdrStateMap &states) override;

  private:
    /**
     * @brief Resolve a library device id to a CoreGraphics display id.
     * @param device_id Device id to resolve.
     * @param query_type Display list type to search.
     * @return Display id, or empty optional if not found.
     */
    [[nodiscard]] std::optional<MacDisplayId> getDisplayId(std::string_view device_id, MacQueryType query_type) const;

    std::shared_ptr<MacApiLayerInterface> m_m_api;
  };
}  // namespace display_device
