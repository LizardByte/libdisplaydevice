/**
 * @file src/macos/include/display_device/macos/mac_api_layer.h
 * @brief Declarations for the MacApiLayer.
 */
#pragma once

// local includes
#include "mac_api_layer_interface.h"

namespace display_device {
  /**
   * @brief Default implementation for the MacApiLayerInterface.
   */
  class MacApiLayer: public MacApiLayerInterface {
  public:
    /**
     * @copydoc MacApiLayerInterface::isApiAccessAvailable
     */
    [[nodiscard]] bool isApiAccessAvailable() const override;

    /**
     * @copydoc MacApiLayerInterface::getErrorString
     */
    [[nodiscard]] std::string getErrorString(MacApiError error_code) const override;

    /**
     * @copydoc MacApiLayerInterface::getDisplayIds
     */
    [[nodiscard]] MacDisplayIdList getDisplayIds(MacQueryType type) const override;

    /**
     * @copydoc MacApiLayerInterface::getDeviceId
     */
    [[nodiscard]] std::string getDeviceId(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getCurrentDisplayMode
     */
    [[nodiscard]] std::optional<MacDisplayMode> getCurrentDisplayMode(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getDisplayModes
     */
    [[nodiscard]] MacDisplayModeList getDisplayModes(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getDisplayName
     */
    [[nodiscard]] std::string getDisplayName(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getFriendlyName
     */
    [[nodiscard]] std::string getFriendlyName(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getEdid
     */
    [[nodiscard]] std::vector<std::byte> getEdid(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getDisplayScale
     */
    [[nodiscard]] std::optional<Rational> getDisplayScale(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getOriginPoint
     */
    [[nodiscard]] std::optional<Point> getOriginPoint(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::isMainDisplay
     */
    [[nodiscard]] bool isMainDisplay(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::isActive
     */
    [[nodiscard]] bool isActive(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::isOnline
     */
    [[nodiscard]] bool isOnline(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::getMirrorMaster
     */
    [[nodiscard]] MacDisplayId getMirrorMaster(MacDisplayId display_id) const override;

    /**
     * @copydoc MacApiLayerInterface::setDisplayMode
     */
    [[nodiscard]] bool setDisplayMode(MacDisplayId display_id, const MacDisplayMode &mode) override;

    /**
     * @copydoc MacApiLayerInterface::setOriginPoint
     */
    [[nodiscard]] bool setOriginPoint(MacDisplayId display_id, const Point &origin) override;

    /**
     * @copydoc MacApiLayerInterface::setMirror
     */
    [[nodiscard]] bool setMirror(MacDisplayId display_id, MacDisplayId master_display_id) override;
  };
}  // namespace display_device
