// local includes
#include "displaydevice/windows/winapilayer.h"
#include "fixtures.h"

TEST(WinApiLayer, GetErrorString) {
  const display_device::WinApiLayer layer;

  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_INVALID_PARAMETER), R"(\[code: ERROR_INVALID_PARAMETER, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_NOT_SUPPORTED), R"(\[code: ERROR_NOT_SUPPORTED, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_ACCESS_DENIED), R"(\[code: ERROR_ACCESS_DENIED, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_INSUFFICIENT_BUFFER), R"(\[code: ERROR_INSUFFICIENT_BUFFER, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_GEN_FAILURE), R"(\[code: ERROR_GEN_FAILURE, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_SUCCESS), R"(\[code: ERROR_SUCCESS, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_ACCOUNT_DISABLED), R"(\[code: )" + std::to_string(ERROR_ACCOUNT_DISABLED) + R"(, .+?\])"));
}

TEST(WinApiLayer, QueryDisplayConfigPathAndModeCount) {
  const display_device::WinApiLayer layer;

  const auto active_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::Active) };
  const auto all_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::All) };

  ASSERT_TRUE(active_devices);
  ASSERT_TRUE(all_devices);

  // This test (and some others) is pointless without any paths. We should always have at least 1 active display device!
  EXPECT_TRUE(!active_devices->paths.empty());
  EXPECT_TRUE(all_devices->paths.size() >= active_devices->paths.size());
  EXPECT_TRUE(all_devices->modes.size() == active_devices->modes.size());
}

TEST(WinApiLayer, QueryDisplayConfigPathActivePaths) {
  const display_device::WinApiLayer layer;

  const auto active_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::Active) };
  ASSERT_TRUE(active_devices);

  for (const auto &path : active_devices->paths) {
    EXPECT_TRUE(static_cast<bool>(path.flags & DISPLAYCONFIG_PATH_ACTIVE));
  }
}

TEST(WinApiLayer, QueryDisplayConfigModeIndexValidity) {
  // The MS docs is not clear when to access the index union struct or not. It appears that union struct is available,
  // whenever QDC_VIRTUAL_MODE_AWARE is specified when querying (always in our case).
  //
  // The docs state, however, that it is only available when DISPLAYCONFIG_PATH_SUPPORT_VIRTUAL_MODE flag is set, but
  // that is just BS (maybe copy-pasta mistake), because some cases were found where the flag is not set and the union
  // is still being used.
  const display_device::WinApiLayer layer;

  const auto active_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::Active) };
  const auto all_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::All) };

  for (const auto &devices : { active_devices, all_devices }) {
    ASSERT_TRUE(devices);

    for (const auto &path : devices->paths) {
      const auto clone_group_id = path.sourceInfo.cloneGroupId;
      const auto source_mode_index = path.sourceInfo.sourceModeInfoIdx;
      const auto target_mode_index = path.targetInfo.targetModeInfoIdx;
      const auto desktop_mode_index = path.targetInfo.desktopModeInfoIdx;

      // It is always invalid unless we are preparing paths for new topology
      EXPECT_EQ(clone_group_id, DISPLAYCONFIG_PATH_CLONE_GROUP_INVALID);

      if (source_mode_index != DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID) {
        ASSERT_TRUE(source_mode_index < devices->modes.size());
        EXPECT_EQ(devices->modes[source_mode_index].infoType, DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE);
      }

      if (target_mode_index != DISPLAYCONFIG_PATH_TARGET_MODE_IDX_INVALID) {
        ASSERT_TRUE(target_mode_index < devices->modes.size());
        EXPECT_EQ(devices->modes[target_mode_index].infoType, DISPLAYCONFIG_MODE_INFO_TYPE_TARGET);
      }

      if (desktop_mode_index != DISPLAYCONFIG_PATH_DESKTOP_IMAGE_IDX_INVALID) {
        ASSERT_TRUE(desktop_mode_index < devices->modes.size());
        EXPECT_EQ(devices->modes[desktop_mode_index].infoType, DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE);
      }
    }
  }
}

TEST(WinApiLayer, GetDeviceId) {
  const display_device::WinApiLayer layer;

  const auto all_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::All) };
  ASSERT_TRUE(all_devices);

  std::map<std::string, std::string> device_id_per_device_path;
  for (const auto &path : all_devices->paths) {
    const auto device_id { layer.get_device_id(path) };
    const auto device_id_2 { layer.get_device_id(path) };
    const auto device_path { layer.get_monitor_device_path(path) };

    // Testing soft persistence - ids remain the same between calls
    EXPECT_EQ(device_id, device_id_2);

    if (device_id.empty()) {
      EXPECT_EQ(path.targetInfo.targetAvailable, FALSE);
    }
    else {
      auto path_it { device_id_per_device_path.find(device_path) };
      if (path_it != std::end(device_id_per_device_path)) {
        // Devices with the same paths must have the same device ids.
        EXPECT_EQ(path_it->second, device_id);
      }
      else {
        EXPECT_TRUE(device_id_per_device_path.insert({ device_path, device_id }).second);
      }
    }
  }
}

TEST(WinApiLayer, GetMonitorDevicePath) {
  const display_device::WinApiLayer layer;

  const auto all_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::All) };
  ASSERT_TRUE(all_devices);

  std::set<std::string> current_device_paths;
  for (const auto &path : all_devices->paths) {
    const auto device_path { layer.get_monitor_device_path(path) };
    const auto device_path_2 { layer.get_monitor_device_path(path) };

    // Testing soft persistence - paths remain the same between calls
    EXPECT_EQ(device_path, device_path_2);

    if (device_path.empty()) {
      EXPECT_EQ(path.targetInfo.targetAvailable, FALSE);
    }
    else if (current_device_paths.contains(device_path)) {
      // In case we have a duplicate device path, the path must be inactive, because
      // active paths are always in the front of the list and therefore will be added
      // first (only 1 active path is possible).
      EXPECT_FALSE(static_cast<bool>(path.flags & DISPLAYCONFIG_PATH_ACTIVE));
    }
    else {
      EXPECT_TRUE(current_device_paths.insert(device_path).second);
    }
  }
}

TEST(WinApiLayer, GetFriendlyName) {
  const display_device::WinApiLayer layer;

  const auto all_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::All) };
  ASSERT_TRUE(all_devices);

  for (const auto &path : all_devices->paths) {
    const auto friendly_name { layer.get_friendly_name(path) };
    const auto friendly_name_2 { layer.get_friendly_name(path) };

    // Testing soft persistence - ids remain the same between calls
    EXPECT_EQ(friendly_name, friendly_name_2);

    // We don't really have anything else to compare the friendly name against without going deep into EDID data.
    // Friendly name is also not mandatory in the EDID...
  }
}

TEST(WinApiLayer, GetDisplayName) {
  const display_device::WinApiLayer layer;

  const auto all_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::All) };
  ASSERT_TRUE(all_devices);

  for (const auto &path : all_devices->paths) {
    const auto display_name { layer.get_display_name(path) };
    const auto display_name_2 { layer.get_display_name(path) };

    // Testing soft persistence - ids remain the same between calls
    EXPECT_EQ(display_name, display_name_2);

    // Display name is attached to the "source" mode (not a physical device) therefore a non-empty
    // value is always expected.
    EXPECT_TRUE(test_regex(display_name, R"(^\\\\.\\DISPLAY\d+$)"));
  }
}
