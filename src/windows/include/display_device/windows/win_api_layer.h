/**
 * @file src/windows/include/display_device/windows/win_api_layer.h
 * @brief Declarations for the WinApiLayer.
 */
#pragma once

// system includes
#include <string_view>

// local includes
#include "win_api_layer_interface.h"

namespace display_device {
  /**
   * @brief Default implementation for the WinApiLayerInterface.
   */
  class WinApiLayer: public WinApiLayerInterface {
  public:
    /**
     * @copydoc WinApiLayerInterface::getErrorString
     */
    [[nodiscard]] std::string getErrorString(LONG error_code) const override;

    /**
     * @copydoc WinApiLayerInterface::queryDisplayConfig
     */
    [[nodiscard]] std::optional<PathAndModeData> queryDisplayConfig(QueryType type) const override;

    /**
     * @copydoc WinApiLayerInterface::wakeDisplay
     */
    [[nodiscard]] bool wakeDisplay(std::chrono::milliseconds timeout) override;

    /**
     * @copydoc WinApiLayerInterface::keepDisplayAwake
     */
    [[nodiscard]] bool keepDisplayAwake() override;

    /**
     * @copydoc WinApiLayerInterface::restorePowerRequest
     */
    [[nodiscard]] bool restorePowerRequest() override;

    /**
     * @copydoc WinApiLayerInterface::getDeviceId
     */
    [[nodiscard]] std::string getDeviceId(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /**
     * @copydoc WinApiLayerInterface::getEdid
     */
    [[nodiscard]] std::vector<std::byte> getEdid(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /**
     * @copydoc WinApiLayerInterface::getMonitorDevicePath
     */
    [[nodiscard]] std::string getMonitorDevicePath(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /**
     * @copydoc WinApiLayerInterface::getFriendlyName
     */
    [[nodiscard]] std::string getFriendlyName(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /**
     * @copydoc WinApiLayerInterface::getDisplayName
     */
    [[nodiscard]] std::string getDisplayName(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /**
     * @copydoc WinApiLayerInterface::setDisplayConfig
     */
    [[nodiscard]] LONG setDisplayConfig(std::vector<DISPLAYCONFIG_PATH_INFO> paths, std::vector<DISPLAYCONFIG_MODE_INFO> modes, UINT32 flags) override;

    /**
     * @copydoc WinApiLayerInterface::getHdrState
     */
    [[nodiscard]] std::optional<HdrState> getHdrState(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /**
     * @copydoc WinApiLayerInterface::setHdrState
     */
    [[nodiscard]] bool setHdrState(const DISPLAYCONFIG_PATH_INFO &path, HdrState state) override;

    /**
     * @copydoc WinApiLayerInterface::getDisplayScale
     */
    [[nodiscard]] std::optional<Rational> getDisplayScale(std::string_view display_name, const DISPLAYCONFIG_SOURCE_MODE &source_mode) const override;
  };
}  // namespace display_device
