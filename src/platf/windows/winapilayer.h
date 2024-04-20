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
  };
}  // namespace display_device
