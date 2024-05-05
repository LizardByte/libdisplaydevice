#pragma once

// local includes
#include "winapilayerinterface.h"

namespace display_device {
  /**
   * @brief Default implementation for the WinApiLayerInterface.
   */
  class WinApiLayer: public WinApiLayerInterface {
  public:
    /** For details @see WinApiLayerInterface::get_error_string */
    [[nodiscard]] std::string
    get_error_string(LONG error_code) const override;

    /** For details @see WinApiLayerInterface::query_display_config */
    [[nodiscard]] std::optional<path_and_mode_data_t>
    query_display_config(query_type_e type) const override;

    /** For details @see WinApiLayerInterface::get_device_id */
    [[nodiscard]] std::string
    get_device_id(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /** For details @see WinApiLayerInterface::get_monitor_device_path */
    [[nodiscard]] std::string
    get_monitor_device_path(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /** For details @see WinApiLayerInterface::get_friendly_name */
    [[nodiscard]] std::string
    get_friendly_name(const DISPLAYCONFIG_PATH_INFO &path) const override;

    /** For details @see WinApiLayerInterface::get_display_name */
    [[nodiscard]] std::string
    get_display_name(const DISPLAYCONFIG_PATH_INFO &path) const override;
  };
}  // namespace display_device
