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
    enum class QueryType {
      Active, /**< The device path must be active. */
      All /**< The device path can be active or inactive. */
    };

    /**
     * @brief Contains currently available paths and associated modes.
     */
    struct PathAndModeData {
      std::vector<DISPLAYCONFIG_PATH_INFO> m_paths; /**< Available display paths. */
      std::vector<DISPLAYCONFIG_MODE_INFO> m_modes; /**< Display modes for ACTIVE displays. */
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
     * const std::string error_message = iface->getErrorString(ERROR_NOT_SUPPORTED);
     * ```
     */
    [[nodiscard]] virtual std::string
    getErrorString(LONG error_code) const = 0;

    /**
     * @brief Query Windows for the device paths and associated modes.
     * @param type Specify device type to query for.
     * @returns Data containing paths and modes, empty optional if we have failed to query.
     *
     * EXAMPLES:
     * ```cpp
     * const WinApiLayerInterface* iface = getIface(...);
     * const auto display_data = iface->queryDisplayConfig(QueryType::Active);
     * ```
     */
    [[nodiscard]] virtual std::optional<PathAndModeData>
    queryDisplayConfig(QueryType type) const = 0;

    /**
     * @brief Get a stable and persistent device id for the path.
     *
     * This function tries to generate a unique id for the path that
     * is persistent between driver re-installs and physical unplugging and
     * replugging of the device.
     *
     * The best candidate for it could have been a "ContainerID" from the
     * registry, however it was found to be unstable for the virtual display
     * (probably because it uses the EDID for the id generation and the current
     * virtual displays have incomplete EDID information). The "ContainerID"
     * also does not change if the physical device is plugged into a different
     * port and seems to be very stable, however because of virtual displays
     * other solution was used.
     *
     * The accepted solution was to use the "InstanceID" and EDID (just to be
     * on the safe side). "InstanceID" is semi-stable, it has some parts that
     * change between driver re-installs and it has a part that changes based
     * on the GPU port that the display is connected to. It is most likely to
     * be unique, but since the MS documentation is lacking we are also hashing
     * EDID information (contains serial ids, timestamps and etc. that should
     * guarantee that identical displays are differentiated like with the
     * "ContainerID"). Most importantly this information is stable for the virtual
     * displays.
     *
     * After we remove the unstable parts from the "InstanceID" and hash everything
     * together, we get an id that changes only when you connect the display to
     * a different GPU port which seems to be acceptable.
     *
     * As a fallback we are using a hashed device path, in case the "InstanceID" or
     * EDID is not available. At least if you don't do driver re-installs often
     * and change the GPU ports, it will be stable for a while.
     *
     * @param path Path to get the device id for.
     * @returns Device id, or an empty string if it could not be generated.
     * @see queryDisplayConfig on how to get paths from the system.
     *
     * EXAMPLES:
     * ```cpp
     * DISPLAYCONFIG_PATH_INFO path;
     * const WinApiLayerInterface* iface = getIface(...);
     * const std::string device_path = iface->getDeviceId(path);
     * ```
     */
    [[nodiscard]] virtual std::string
    getDeviceId(const DISPLAYCONFIG_PATH_INFO &path) const = 0;

    /**
     * @brief Get a string that represents a path from the adapter to the display target.
     * @param path Path to get the string for.
     * @returns String representation, or an empty string if it's not available.
     * @see queryDisplayConfig on how to get paths from the system.
     * @note In the rest of the code we refer to this string representation simply as the "device path".
     *       It is used as a simple way of grouping related path objects together and removing "bad" paths
     *       that don't have such string representation.
     *
     * EXAMPLES:
     * ```cpp
     * DISPLAYCONFIG_PATH_INFO path;
     * const WinApiLayerInterface* iface = getIface(...);
     * const std::string device_path = iface->getMonitorDevicePath(path);
     * ```
     */
    [[nodiscard]] virtual std::string
    getMonitorDevicePath(const DISPLAYCONFIG_PATH_INFO &path) const = 0;

    /**
     * @brief Get the user friendly name for the path.
     * @param path Path to get user friendly name for.
     * @returns User friendly name for the path if available, empty string otherwise.
     * @see queryDisplayConfig on how to get paths from the system.
     * @note This is usually a monitor name (like "ROG PG279Q") and is most likely taken from EDID.
     *
     * EXAMPLES:
     * ```cpp
     * DISPLAYCONFIG_PATH_INFO path;
     * const WinApiLayerInterface* iface = getIface(...);
     * const std::string friendly_name = iface->getFriendlyName(path);
     * ```
     */
    [[nodiscard]] virtual std::string
    getFriendlyName(const DISPLAYCONFIG_PATH_INFO &path) const = 0;

    /**
     * @brief Get the logical display name for the path.
     *
     * These are the "\\\\.\\DISPLAY1", "\\\\.\\DISPLAY2" and etc. display names that can
     * change whenever Windows wants to change them.
     *
     * @param path Path to get user display name for.
     * @returns Display name for the path if available, empty string otherwise.
     * @see queryDisplayConfig on how to get paths from the system.
     * @note Inactive paths can have these names already assigned to them, even
     *       though they are not even in use! There can also be duplicates.
     *
     * EXAMPLES:
     * ```cpp
     * DISPLAYCONFIG_PATH_INFO path;
     * const WinApiLayerInterface* iface = getIface(...);
     * const std::string display_name = iface->getDisplayName(path);
     * ```
     */
    [[nodiscard]] virtual std::string
    getDisplayName(const DISPLAYCONFIG_PATH_INFO &path) const = 0;
  };
}  // namespace display_device
