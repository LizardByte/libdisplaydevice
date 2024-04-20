#pragma once

// the most stupid and smelly windows include
#include <windows.h>

// system includes
#include <optional>
#include <string>
#include <vector>

namespace display_device {
  /**
   * @brief Lowest level Windows API wrapper for easy mocking.
   */
  class WinApiLayerInterface {
  public:
    /**
     * @brief Type of query the OS should perform while searching for display devices.
     */
    enum class query_type_e {
      Active, /**< The device path must be active. */
      All /**< The device path can be active or inactive. */
    };

    /**
     * @brief Contains currently available paths and associated modes.
     */
    struct path_and_mode_data_t {
      std::vector<DISPLAYCONFIG_PATH_INFO> paths; /**< Available display paths. */
      std::vector<DISPLAYCONFIG_MODE_INFO> modes; /**< Display modes for ACTIVE displays. */
    };

    /**
     * @brief Default virtual destructor.
     */
    virtual ~WinApiLayerInterface() = default;

    /**
     * @brief Stringify the error code from Windows API.
     * @param error_code Error code to stringify.
     * @returns String containing the error code in a readable format + a system message describing the code.
     *
     * EXAMPLES:
     * ```cpp
     * const WinApiLayerInterface* iface = getIface(...);
     * const std::string error_message = iface->get_error_string(ERROR_NOT_SUPPORTED);
     * ```
     */
    [[nodiscard]] virtual std::string
    get_error_string(LONG error_code) const = 0;

    /**
     * @brief Query Windows for the device paths and associated modes.
     * @param type Specify device type to query for.
     * @returns Data containing paths and modes, empty optional if we have failed to query.
     *
     * EXAMPLES:
     * ```cpp
     * const WinApiLayerInterface* iface = getIface(...);
     * const auto display_data = iface->query_display_config(query_type_e::Active);
     * ```
     */
    [[nodiscard]] virtual std::optional<path_and_mode_data_t>
    query_display_config(query_type_e type) const = 0;
  };
}  // namespace display_device
