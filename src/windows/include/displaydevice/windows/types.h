#pragma once

// the most stupid and smelly windows include
#include <windows.h>

// system includes
#include <functional>
#include <map>
#include <set>

// local includes
#include "displaydevice/types.h"

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
    std::vector<DISPLAYCONFIG_PATH_INFO> m_paths {}; /**< Available display paths. */
    std::vector<DISPLAYCONFIG_MODE_INFO> m_modes {}; /**< Display modes for ACTIVE displays. */
  };

  /**
   * @brief Specifies additional constraints for the validated device.
   */
  enum class ValidatedPathType {
    Active, /**< The device path must be active. */
    Any /**< The device path can be active or inactive. */
  };

  /**
   * @brief Contains the device path and the id for a VALID device.
   * @see win_utils::getDeviceInfoForValidPath for what is considered a valid device.
   * @see WinApiLayerInterface::getDeviceId for how we make the device id.
   */
  struct ValidatedDeviceInfo {
    std::string m_device_path {}; /**< Unique device path string. */
    std::string m_device_id {}; /**< A device id (made up by us) that is identifies the device. */
  };

  /**
   * @brief Contains information about sources with identical adapter ids from matching paths.
   */
  struct PathSourceIndexData {
    std::map<UINT32, std::size_t> m_source_id_to_path_index {}; /**< Maps source ids to its index in the path list. */
    LUID m_adapter_id {}; /**< Adapter id shared by all source ids. */
    std::optional<UINT32> m_active_source {}; /**< Currently active source id. */
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

  /**
   * @brief Display's mode (resolution + refresh rate).
   */
  struct DisplayMode {
    Resolution m_resolution {};
    Rational m_refresh_rate {};

    /**
     * @brief Comparator for strict equality.
     */
    friend bool
    operator==(const DisplayMode &lhs, const DisplayMode &rhs);
  };

  /**
   * @brief Ordered map of [DEVICE_ID -> DisplayMode].
   */
  using DeviceDisplayModeMap = std::map<std::string, DisplayMode>;

  /**
   * @brief Ordered map of [DEVICE_ID -> std::optional<HdrState>].
   */
  using HdrStateMap = std::map<std::string, std::optional<HdrState>>;

  /**
   * @brief Arbitrary data for making and undoing changes.
   */
  struct SingleDisplayConfigState {
    /**
     * @brief Data that represents the original system state and is used
     *        as a base when trying to re-apply settings without reverting settings.
     */
    struct Initial {
      ActiveTopology m_topology {};
      std::set<std::string> m_primary_devices {};

      /**
       * @brief Comparator for strict equality.
       */
      friend bool
      operator==(const Initial &lhs, const Initial &rhs);
    };

    /**
     * @brief Data for tracking the modified changes.
     */
    struct Modified {
      ActiveTopology m_topology {};
      DeviceDisplayModeMap m_original_modes {};
      HdrStateMap m_original_hdr_states {};
      std::string m_original_primary_device {};

      /**
       * @brief Check if the changed topology has any other modifications.
       * @return True if DisplayMode, HDR or primary device has been changed, false otherwise.
       *
       * EXAMPLES:
       * ```cpp
       * SingleDisplayConfigState state;
       * const no_modifications = state.hasModifications();
       *
       * state.modified.original_primary_device = "DeviceId2";
       * const has_modifications = state.hasModifications();
       * ```
       */
      [[nodiscard]] bool
      hasModifications() const;

      /**
       * @brief Comparator for strict equality.
       */
      friend bool
      operator==(const Modified &lhs, const Modified &rhs);
    };

    Initial m_initial;
    Modified m_modified;

    /**
     * @brief Comparator for strict equality.
     */
    friend bool
    operator==(const SingleDisplayConfigState &lhs, const SingleDisplayConfigState &rhs);
  };

  /**
   * @brief Default function type used for cleanup/guard functions.
   */
  using DdGuardFn = std::function<void()>;
}  // namespace display_device
