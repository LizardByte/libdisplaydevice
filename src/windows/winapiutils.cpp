// header include
#include "displaydevice/windows/winapiutils.h"

// local includes
#include "displaydevice/logging.h"

namespace {
  /**
   * @brief Check if adapter ids are NOT equal.
   * @param lhs First id to check.
   * @param rhs Second id to check.
   * @return True if NOT equal, false otherwise.
   *
   * EXAMPLES:
   * ```cpp
   * const bool not_equal = (LUID{ 12, 34 }) != (LUID{ 12, 56 });
   * ```
   */
  bool
  operator!=(const LUID &lhs, const LUID &rhs) {
    return lhs.HighPart != rhs.HighPart || lhs.LowPart != rhs.LowPart;
  }
}  // namespace

namespace display_device::win_utils {
  bool
  isAvailable(const DISPLAYCONFIG_PATH_INFO &path) {
    return path.targetInfo.targetAvailable == TRUE;
  }

  bool
  isActive(const DISPLAYCONFIG_PATH_INFO &path) {
    return static_cast<bool>(path.flags & DISPLAYCONFIG_PATH_ACTIVE);
  }

  std::optional<UINT32>
  getSourceIndex(const DISPLAYCONFIG_PATH_INFO &path, const std::vector<DISPLAYCONFIG_MODE_INFO> &modes) {
    // The MS docs is not clear when to access the index union struct or not. It appears that union struct is available,
    // whenever QDC_VIRTUAL_MODE_AWARE is specified when querying (always in our case).
    //
    // The docs state, however, that it is only available when DISPLAYCONFIG_PATH_SUPPORT_VIRTUAL_MODE flag is set, but
    // that is just BS (maybe copy-pasta mistake), because some cases were found where the flag is not set and the union
    // is still being used.

    const UINT32 index { path.sourceInfo.sourceModeInfoIdx };
    if (index == DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID) {
      return std::nullopt;
    }

    if (index >= modes.size()) {
      DD_LOG(error) << "Source index " << index << " is out of range " << modes.size();
      return std::nullopt;
    }

    return index;
  }

  void
  setSourceIndex(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &index) {
    // The MS docs is not clear when to access the index union struct or not. It appears that union struct is available,
    // whenever QDC_VIRTUAL_MODE_AWARE is specified when querying (always in our case).
    //
    // The docs state, however, that it is only available when DISPLAYCONFIG_PATH_SUPPORT_VIRTUAL_MODE flag is set, but
    // that is just BS (maybe copy-pasta mistake), because some cases were found where the flag is not set and the union
    // is still being used.

    if (index) {
      path.sourceInfo.sourceModeInfoIdx = *index;
    }
    else {
      path.sourceInfo.sourceModeInfoIdx = DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID;
    }
  }
  void
  setTargetIndex(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &index) {
    // The MS docs is not clear when to access the index union struct or not. It appears that union struct is available,
    // whenever QDC_VIRTUAL_MODE_AWARE is specified when querying (always in our case).
    //
    // The docs state, however, that it is only available when DISPLAYCONFIG_PATH_SUPPORT_VIRTUAL_MODE flag is set, but
    // that is just BS (maybe copy-pasta mistake), because some cases were found where the flag is not set and the union
    // is still being used.

    if (index) {
      path.targetInfo.targetModeInfoIdx = *index;
    }
    else {
      path.targetInfo.targetModeInfoIdx = DISPLAYCONFIG_PATH_TARGET_MODE_IDX_INVALID;
    }
  }
  void
  setDesktopIndex(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &index) {
    // The MS docs is not clear when to access the index union struct or not. It appears that union struct is available,
    // whenever QDC_VIRTUAL_MODE_AWARE is specified when querying (always in our case).
    //
    // The docs state, however, that it is only available when DISPLAYCONFIG_PATH_SUPPORT_VIRTUAL_MODE flag is set, but
    // that is just BS (maybe copy-pasta mistake), because some cases were found where the flag is not set and the union
    // is still being used.

    if (index) {
      path.targetInfo.desktopModeInfoIdx = *index;
    }
    else {
      path.targetInfo.desktopModeInfoIdx = DISPLAYCONFIG_PATH_DESKTOP_IMAGE_IDX_INVALID;
    }
  }
  void
  setCloneGroupId(DISPLAYCONFIG_PATH_INFO &path, const std::optional<UINT32> &id) {
    // The MS docs is not clear when to access the index union struct or not. It appears that union struct is available,
    // whenever QDC_VIRTUAL_MODE_AWARE is specified when querying (always in our case).
    //
    // The docs state, however, that it is only available when DISPLAYCONFIG_PATH_SUPPORT_VIRTUAL_MODE flag is set, but
    // that is just BS (maybe copy-pasta mistake), because some cases were found where the flag is not set and the union
    // is still being used.

    if (id) {
      path.sourceInfo.cloneGroupId = *id;
    }
    else {
      path.sourceInfo.cloneGroupId = DISPLAYCONFIG_PATH_CLONE_GROUP_INVALID;
    }
  }

  const DISPLAYCONFIG_SOURCE_MODE *
  getSourceMode(const std::optional<UINT32> &index, const std::vector<DISPLAYCONFIG_MODE_INFO> &modes) {
    if (!index) {
      return nullptr;
    }

    if (*index >= modes.size()) {
      DD_LOG(error) << "Source index " << *index << " is out of range " << modes.size();
      return nullptr;
    }

    const auto &mode { modes[*index] };
    if (mode.infoType != DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {
      DD_LOG(error) << "Mode at index " << *index << " is not source mode!";
      return nullptr;
    }

    return &mode.sourceMode;
  }

  DISPLAYCONFIG_SOURCE_MODE *
  getSourceMode(const std::optional<UINT32> &index, std::vector<DISPLAYCONFIG_MODE_INFO> &modes) {
    return const_cast<DISPLAYCONFIG_SOURCE_MODE *>(getSourceMode(index, const_cast<const std::vector<DISPLAYCONFIG_MODE_INFO> &>(modes)));
  }

  std::optional<ValidatedDeviceInfo>
  getDeviceInfoForValidPath(const WinApiLayerInterface &w_api, const DISPLAYCONFIG_PATH_INFO &path, bool must_be_active) {
    if (!isAvailable(path)) {
      // Could be transient issue according to MSDOCS (no longer available, but still "active")
      return std::nullopt;
    }

    if (must_be_active) {
      if (!isActive(path)) {
        return std::nullopt;
      }
    }

    const auto device_path { w_api.getMonitorDevicePath(path) };
    if (device_path.empty()) {
      return std::nullopt;
    }

    const auto device_id { w_api.getDeviceId(path) };
    if (device_id.empty()) {
      return std::nullopt;
    }

    const auto display_name { w_api.getDisplayName(path) };
    if (display_name.empty()) {
      return std::nullopt;
    }

    return ValidatedDeviceInfo { device_path, device_id };
  }

  PathSourceIndexDataMap
  collectSourceDataForMatchingPaths(const WinApiLayerInterface &w_api, const std::vector<DISPLAYCONFIG_PATH_INFO> &paths) {
    PathSourceIndexDataMap path_data;

    std::unordered_map<std::string, std::string> paths_to_ids;
    for (std::size_t index = 0; index < paths.size(); ++index) {
      const auto &path { paths[index] };

      const auto device_info { getDeviceInfoForValidPath(w_api, path, false) };
      if (!device_info) {
        // Path is not valid
        continue;
      }

      const auto prev_device_id_for_path_it { paths_to_ids.find(device_info->m_device_path) };
      if (prev_device_id_for_path_it != std::end(paths_to_ids)) {
        if (prev_device_id_for_path_it->second != device_info->m_device_id) {
          DD_LOG(error) << "Duplicate display device id found: " << device_info->m_device_id << " (device path: " << device_info->m_device_path << ")";
          return {};
        }
      }
      else {
        for (const auto &[device_path, device_id] : paths_to_ids) {
          if (device_id == device_info->m_device_id) {
            DD_LOG(error) << "Device id " << device_info->m_device_id << " is shared between 2 different paths: " << device_path << " and " << device_info->m_device_path;
            return {};
          }
        }

        paths_to_ids[device_info->m_device_path] = device_info->m_device_id;
      }

      auto path_data_it { path_data.find(device_info->m_device_id) };
      if (path_data_it != std::end(path_data)) {
        if (path_data_it->second.m_adapter_id != path.sourceInfo.adapterId) {
          // Sanity check, should not be possible since adapter in embedded in the device path
          DD_LOG(error) << "Device path " << device_info->m_device_path << " has different adapters!";
          return {};
        }
        else if (isActive(path)) {
          // Sanity check, should not be possible as all active paths are in the front
          DD_LOG(error) << "Device path " << device_info->m_device_path << " is active, but not the first entry in the list!";
          return {};
        }
        else if (path_data_it->second.m_source_id_to_path_index.contains(path.sourceInfo.id)) {
          // Sanity check, should not be possible unless Windows goes bonkers
          DD_LOG(error) << "Device path " << device_info->m_device_path << " has duplicate source ids!";
          return {};
        }

        path_data_it->second.m_source_id_to_path_index[path.sourceInfo.id] = index;
      }
      else {
        path_data[device_info->m_device_id] = {
          { { path.sourceInfo.id, index } },
          path.sourceInfo.adapterId,
          // Since active paths are always in the front, this is the only time we set it
          isActive(path) ? std::make_optional(path.sourceInfo.id) : std::nullopt
        };
      }

      DD_LOG(verbose) << "Device " << device_info->m_device_id << " (active: " << isActive(path) << ") at index " << index << " added to the source data list.";
    }

    if (path_data.empty()) {
      DD_LOG(error) << "Failed to collect path source data or none was available!";
    }
    return path_data;
  }
}  // namespace display_device::win_utils
