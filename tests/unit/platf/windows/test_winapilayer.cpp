// local includes
#include "src/platf/windows/winapilayer.h"
#include "tests/conftest.cpp"
#include "tests/utils.h"

TEST(LoggingTest, GetErrorString) {
  const display_device::WinApiLayer layer;

  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_INVALID_PARAMETER), R"(\[code: ERROR_INVALID_PARAMETER, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_NOT_SUPPORTED), R"(\[code: ERROR_NOT_SUPPORTED, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_ACCESS_DENIED), R"(\[code: ERROR_ACCESS_DENIED, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_INSUFFICIENT_BUFFER), R"(\[code: ERROR_INSUFFICIENT_BUFFER, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_GEN_FAILURE), R"(\[code: ERROR_GEN_FAILURE, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_SUCCESS), R"(\[code: ERROR_SUCCESS, .+?\])"));
  EXPECT_TRUE(test_regex(layer.get_error_string(ERROR_ACCOUNT_DISABLED), R"(\[code: )" + std::to_string(ERROR_ACCOUNT_DISABLED) + R"(, .+?\])"));
}

TEST(LoggingTest, QueryDisplayConfigPathAndModeCount) {
  const display_device::WinApiLayer layer;

  const auto active_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::Active) };
  const auto all_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::All) };

  EXPECT_TRUE(active_devices);
  EXPECT_TRUE(all_devices);
  EXPECT_TRUE(all_devices->paths.size() >= active_devices->paths.size());
  EXPECT_TRUE(all_devices->modes.size() == active_devices->modes.size());
}

TEST(LoggingTest, QueryDisplayConfigPathActivePaths) {
  const display_device::WinApiLayer layer;

  const auto active_devices { layer.query_display_config(display_device::WinApiLayer::query_type_e::Active) };
  EXPECT_TRUE(active_devices);

  for (const auto &path : active_devices->paths) {
    EXPECT_TRUE(static_cast<bool>(path.flags & DISPLAYCONFIG_PATH_ACTIVE));
  }
}

TEST(LoggingTest, QueryDisplayConfigModeIndexValidity) {
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
    EXPECT_TRUE(devices);

    for (const auto &path : devices->paths) {
      const auto clone_group_id = path.sourceInfo.cloneGroupId;
      const auto source_mode_index = path.sourceInfo.sourceModeInfoIdx;
      const auto target_mode_index = path.targetInfo.targetModeInfoIdx;
      const auto desktop_mode_index = path.targetInfo.desktopModeInfoIdx;

      // It is always invalid unless we are preparing paths for new topology
      EXPECT_EQ(clone_group_id, DISPLAYCONFIG_PATH_CLONE_GROUP_INVALID);

      if (source_mode_index != DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID) {
        EXPECT_TRUE(source_mode_index < devices->modes.size());
        EXPECT_EQ(devices->modes[source_mode_index].infoType, DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE);
      }

      if (target_mode_index != DISPLAYCONFIG_PATH_TARGET_MODE_IDX_INVALID) {
        EXPECT_TRUE(target_mode_index < devices->modes.size());
        EXPECT_EQ(devices->modes[target_mode_index].infoType, DISPLAYCONFIG_MODE_INFO_TYPE_TARGET);
      }

      if (desktop_mode_index != DISPLAYCONFIG_PATH_DESKTOP_IMAGE_IDX_INVALID) {
        EXPECT_TRUE(desktop_mode_index < devices->modes.size());
        EXPECT_EQ(devices->modes[desktop_mode_index].infoType, DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE);
      }
    }
  }
}