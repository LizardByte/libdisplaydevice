// local includes
#include "mockwinapilayer.h"

namespace {
  display_device::PathAndModeData
  make3ActiveDeviceGroups(const bool include_duplicate) {
    display_device::PathAndModeData data;

    // 1st group (1 device)
    {
      data.m_paths.push_back({});
      data.m_paths.back().flags = DISPLAYCONFIG_PATH_ACTIVE;
      data.m_paths.back().sourceInfo.sourceModeInfoIdx = data.m_modes.size();
      data.m_paths.back().targetInfo.targetAvailable = TRUE;

      data.m_modes.push_back({});
      data.m_modes.back().infoType = DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE;
      data.m_modes.back().sourceMode = {};  // Set the union
      data.m_modes.back().sourceMode.position = { 0, 0 };
      data.m_modes.back().sourceMode.width = 1920;
      data.m_modes.back().sourceMode.height = 1080;
    }

    // 2nd group (1+ device)
    {
      data.m_paths.push_back({});
      data.m_paths.back().flags = DISPLAYCONFIG_PATH_ACTIVE;
      data.m_paths.back().sourceInfo.sourceModeInfoIdx = data.m_modes.size();
      data.m_paths.back().targetInfo.targetAvailable = TRUE;

      data.m_modes.push_back({});
      data.m_modes.back().infoType = DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE;
      data.m_modes.back().sourceMode = {};  // Set the union
      data.m_modes.back().sourceMode.position = { 1921, 0 };  // TODO
      data.m_modes.back().sourceMode.width = 1920;
      data.m_modes.back().sourceMode.height = 1080;

      if (include_duplicate) {
        data.m_paths.push_back({});
        data.m_paths.back().flags = DISPLAYCONFIG_PATH_ACTIVE;
        data.m_paths.back().sourceInfo.sourceModeInfoIdx = data.m_modes.size();
        data.m_paths.back().targetInfo.targetAvailable = TRUE;

        data.m_modes.push_back({});
        data.m_modes.back().infoType = DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE;
        data.m_modes.back().sourceMode = {};  // Set the union
        data.m_modes.back().sourceMode.position = { 1921, 0 };
        data.m_modes.back().sourceMode.width = 1920;
        data.m_modes.back().sourceMode.height = 1080;
      }
    }

    // 3rd group (1 device)
    {
      data.m_paths.push_back({});
      data.m_paths.back().flags = DISPLAYCONFIG_PATH_ACTIVE;
      data.m_paths.back().sourceInfo.sourceModeInfoIdx = data.m_modes.size();
      data.m_paths.back().targetInfo.targetAvailable = TRUE;

      data.m_modes.push_back({});
      data.m_modes.back().infoType = DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE;
      data.m_modes.back().sourceMode = {};  // Set the union
      data.m_modes.back().sourceMode.position = { 3842, 0 };
      data.m_modes.back().sourceMode.width = 1920;
      data.m_modes.back().sourceMode.height = 1080;
    }

    return data;
  }
}  // namespace

namespace ut_consts {
  const std::optional<display_device::PathAndModeData> PAM_NULL { std::nullopt };
  const std::optional<display_device::PathAndModeData> PAM_EMPTY {};
  const std::optional<display_device::PathAndModeData> PAM_3_ACTIVE { make3ActiveDeviceGroups(false) };
  const std::optional<display_device::PathAndModeData> PAM_3_ACTIVE_WITH_INVALID_MODE_IDX { []() {
    auto data { make3ActiveDeviceGroups(false) };

    // Scramble the indexes
    auto invalid_idx { DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID };
    for (auto &item : data.m_paths) {
      item.sourceInfo.sourceModeInfoIdx = invalid_idx--;
    }

    return data;
  }() };
  const std::optional<display_device::PathAndModeData> PAM_4_ACTIVE_WITH_2_DUPLICATES { make3ActiveDeviceGroups(true) };
}  // namespace ut_consts
