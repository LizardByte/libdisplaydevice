#pragma once

// the most stupid and smelly windows include
#include <windows.h>

// system includes
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace display_device {
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
   * @brief Contains the device path and the id for a VALID device.
   * @see win_utils::getDeviceInfoForValidPath for what is considered a valid device.
   * @see WinApiLayerInterface::getDeviceId for how we make the device id.
   */
  struct ValidatedDeviceInfo {
    std::string m_device_path; /**< Unique device path string. */
    std::string m_device_id; /**< A device id (made up by us) that is identifies the device. */
  };

  /**
   * @brief Contains information about sources with identical adapter ids from matching paths.
   */
  struct PathSourceIndexData {
    std::unordered_map<UINT32, std::size_t> m_source_id_to_path_index; /**< Maps source ids to its index in the path list. */
    LUID m_adapter_id {}; /**< Adapter id shared by all source ids. */
    std::optional<UINT32> m_active_source; /**< Currently active source id. */
  };

  /**
   * @brief Ordered map of [DEVICE_ID -> PathSourceIndexData].
   * @see PathSourceIndexData
   */
  using PathSourceIndexDataMap = std::map<std::string, PathSourceIndexData>;

  /**
   * @brief A LIST[LIST[DEVICE_ID]] structure which represents an active topology.
   *
   * Single display:
   *     [[DISPLAY_1]]
   * 2 extended displays:
   *     [[DISPLAY_1], [DISPLAY_2]]
   * 2 duplicated displays:
   *     [[DISPLAY_1, DISPLAY_2]]
   * Mixed displays:
   *     [[EXTENDED_DISPLAY_1], [DUPLICATED_DISPLAY_1, DUPLICATED_DISPLAY_2], [EXTENDED_DISPLAY_2]]
   *
   * @note On Windows the order does not matter of both device ids or the inner lists.
   */
  using ActiveTopology = std::vector<std::vector<std::string>>;

}  // namespace display_device
