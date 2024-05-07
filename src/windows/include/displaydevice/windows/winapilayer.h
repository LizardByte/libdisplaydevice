#pragma once

// local includes
#include "winapilayerinterface.h"

namespace display_device {
  /**
   * @brief Default implementation for the WinApiLayerInterface.
   */
  class WinApiLayer: public WinApiLayerInterface {
  public:
    /** For details @see WinApiLayerInterface::getErrorString */
    [[nodiscard]] std::string
    getErrorString(LONG error_code) const override;

    /** For details @see WinApiLayerInterface::queryDisplayConfig */
    [[nodiscard]] std::optional<PathAndModeData>
    queryDisplayConfig(QueryType type) const override;

    /** For details @see WinApiLayerInterface::getDeviceId */
    [[nodiscard]] std::string
    getDeviceId(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /** For details @see WinApiLayerInterface::getMonitorDevicePath */
    [[nodiscard]] std::string
    getMonitorDevicePath(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /** For details @see WinApiLayerInterface::getFriendlyName */
    [[nodiscard]] std::string
    getFriendlyName(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /** For details @see WinApiLayerInterface::getDisplayName */
    [[nodiscard]] std::string
    getDisplayName(const DISPLAYCONFIG_PATH_INFO &path) const override;
  };
}  // namespace display_device
