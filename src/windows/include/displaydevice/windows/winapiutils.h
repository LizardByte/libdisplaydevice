#pragma once

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
   * @brief Mark the display device path as active.
   * @param path Path to mark.
   * @see WinApiLayerInterface::queryDisplayConfig on how to get paths from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * if (!isActive(path)) {
   *   setActive(path);
   * }
   * ```
   */
  void
  setActive(DISPLAYCONFIG_PATH_INFO &path);

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
   * @brief Set the source mode index in the path.
   * @param path Path to modify.
   * @param index Index value to set or empty optional to mark the index as invalid.
   * @see query_display_config on how to get paths and modes from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * set_source_index(path, 5);
   * set_source_index(path, std::nullopt);
   * ```
   */
  void
  setSourceIndex(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &index);

  /**
   * @brief Set the target mode index in the path.
   * @param path Path to modify.
   * @param index Index value to set or empty optional to mark the index as invalid.
   * @see query_display_config on how to get paths and modes from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * set_target_index(path, 5);
   * set_target_index(path, std::nullopt);
   * ```
   */
  void
  setTargetIndex(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &index);

  /**
   * @brief Set the desktop mode index in the path.
   * @param path Path to modify.
   * @param index Index value to set or empty optional to mark the index as invalid.
   * @see query_display_config on how to get paths and modes from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * set_desktop_index(path, 5);
   * set_desktop_index(path, std::nullopt);
   * ```
   */
  void
  setDesktopIndex(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &index);

  /**
   * @brief Set the clone group id in the path.
   * @param path Path to modify.
   * @param id Id value to set or empty optional to mark the id as invalid.
   * @see query_display_config on how to get paths and modes from the system.
   *
   * EXAMPLES:
   * ```cpp
   * DISPLAYCONFIG_PATH_INFO path;
   * set_clone_group_id(path, 5);
   * set_clone_group_id(path, std::nullopt);
   * ```
   */
  void
  setCloneGroupId(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &id);

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

  /**
   * @brief Collect arbitrary source data from provided paths.
   *
   * This function filters paths that can be used later on and
   * collects for a quick lookup.
   *
   * @param paths List of paths.
   * @returns Data for valid paths.
   * @see WinApiLayerInterface::queryDisplayConfig on how to get paths from the system.
   *
   * EXAMPLES:
   * ```cpp
   * std::vector<DISPLAYCONFIG_PATH_INFO> paths;
   * const WinApiLayerInterface* iface = getIface(...);
   * const auto path_data = collectSourceDataForMatchingPaths(*iface, paths);
   * ```
   */
  [[nodiscard]] PathSourceIndexDataMap
  collectSourceDataForMatchingPaths(const WinApiLayerInterface &w_api, const std::vector<DISPLAYCONFIG_PATH_INFO> &paths);

  /**
   * @brief Select the best possible paths to be used for the requested topology based on the data that is available to us.
   *
   * If the paths will be used for a completely new topology (Windows has never had it set), we need to take into
   * account the source id availability per the adapter - duplicated displays must share the same source id
   * (if they belong to the same adapter) and have different ids if they are not duplicated displays.
   *
   * There are limited amount of available ids (see comments in the code) so we will abort early if we are
   * out of ids.
   *
   * The paths for a topology that already exists (Windows has set it at least once) does not have to follow
   * the mentioned "source id" rule. Windows can simply ignore them (we will ask it later) and select
   * paths that were previously configured (that might differ in source ids) based on the paths that we provide.
   *
   * @param new_topology Topology that we want to have in the end.
   * @param path_source_data Collected source data from paths.
   * @param paths Display paths that were used for collecting source data.
   * @return A list of path that will make up new topology, or an empty list if function fails.
   */
  [[nodiscard]] std::vector<DISPLAYCONFIG_PATH_INFO>
  makePathsForNewTopology(const ActiveTopology &new_topology, const PathSourceIndexDataMap &path_source_data, const std::vector<DISPLAYCONFIG_PATH_INFO> &paths);
}  // namespace display_device::win_utils
