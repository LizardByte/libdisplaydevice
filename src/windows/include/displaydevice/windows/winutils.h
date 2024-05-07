#pragma once

// system includes
#include <optional>

// local includes
#include "winapilayerinterface.h"

/**
 * @brief Shared "utility-level" code for Windows.
 */
namespace display_device::win_utils {
  /**
   * @brief Check if the display device path's target is available.
   *
   * In most cases this this would mean physically connected to the system,
   * but it also possible force the path to persist. It is not clear if it be
   * counted as available or not.
   *
   * @param path Path to check.
   * @returns True if path's target is marked as available, false otherwise.
   * @see WinApiLayerInterface::queryDisplayConfig on how to get paths from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * const bool available = isAvailable(path);
   * ```
   */
  [[nodiscard]] bool
  isAvailable(const DISPLAYCONFIG_PATH_INFO &path);

  /**
   * @brief Check if the display device path is marked as active.
   * @param path Path to check.
   * @returns True if path is marked as active, false otherwise.
   * @see WinApiLayerInterface::queryDisplayConfig on how to get paths from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * const bool active = isActive(path);
   * ```
   */
  [[nodiscard]] bool
  isActive(const DISPLAYCONFIG_PATH_INFO &path);

  /**
   * @brief Get the source mode index from the path.
   *
   * It performs sanity checks on the modes list that the index is indeed correct.
   *
   * @param path Path to get the source mode index for.
   * @param modes A list of various modes (source, target, desktop and probably more in the future).
   * @returns Valid index value if it's found in the modes list and the mode at that index is of a type "source" mode,
   *          empty optional otherwise.
   * @see WinApiLayerInterface::queryDisplayConfig on how to get paths and modes from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * std::vector<DISPLAYCONFIG_MODE_INFO> modes;
   * const auto source_index = getSourceIndex(path, modes);
   * ```
   */
  [[nodiscard]] std::optional<UINT32>
  getSourceIndex(const DISPLAYCONFIG_PATH_INFO &path, const std::vector<DISPLAYCONFIG_MODE_INFO> &modes);

  /**
   * @brief Get the source mode from the list at the specified index.
   *
   * This function does additional sanity checks for the modes list and ensures
   * that the mode at the specified index is indeed a source mode.
   *
   * @param index Index to get the mode for. It is of boost::optional type
   *              as the function is intended to be used with getSourceIndex function.
   * @param modes List to get the mode from.
   * @returns A pointer to a valid source mode from to list at the specified index, nullptr otherwise.
   * @see WinApiLayerInterface::queryDisplayConfig on how to get paths and modes from the system.
   * @see getSourceIndex
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * const std::vector<DISPLAYCONFIG_MODE_INFO> modes;
   * const DISPLAYCONFIG_SOURCE_MODE* source_mode = getSourceMode(getSourceIndex(path, modes), modes);
   * ```
   */
  [[nodiscard]] const DISPLAYCONFIG_SOURCE_MODE *
  getSourceMode(const std::optional<UINT32> &index, const std::vector<DISPLAYCONFIG_MODE_INFO> &modes);

  /**
   * @see getSourceMode (const version) for the description.
   */
  [[nodiscard]] DISPLAYCONFIG_SOURCE_MODE *
  getSourceMode(const std::optional<UINT32> &index, std::vector<DISPLAYCONFIG_MODE_INFO> &modes);

  /**
   * @brief Validate the path and get the commonly used information from it.
   *
   * This a convenience function to ensure that our concept of "valid path" remains the
   * same throughout the code.
   *
   * Currently, for use, a valid path is:
   *   - a path with and available display target;
   *   - a path that is active (optional);
   *   - a path that has a non-empty device path;
   *   - a path that has a non-empty device id;
   *   - a path that has a non-empty device name assigned.
   *
   * @param w_api Reference to the Windows API layer.
   * @param path Path to validate and get info for.
   * @param must_be_active Optionally request that the valid path must also be active.
   * @returns Commonly used info for the path, or empty optional if the path is invalid.
   * @see WinApiLayerInterface::queryDisplayConfig on how to get paths and modes from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * const WinApiLayerInterface* iface = getIface(...);
   * const auto device_info = getDeviceInfoForValidPath(*iface, path, true);
   * ```
   */
  [[nodiscard]] std::optional<ValidatedDeviceInfo>
  getDeviceInfoForValidPath(const WinApiLayerInterface &w_api, const DISPLAYCONFIG_PATH_INFO &path, bool must_be_active);
}  // namespace display_device::win_utils
