// header include
#include "displaydevice/windows/winutils.h"

// local includes
#include "displaydevice/logging.h"

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
}  // namespace display_device::win_utils
