// local includes
#include "displaydevice/windows/winapiutils.h"
#include "fixtures.h"
#include "mocks/mockwinapilayer.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class WinApiUtilsMocked: public BaseTest {
  public:
    StrictMock<display_device::MockWinApiLayer> m_layer;
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, WinApiUtilsMocked, __VA_ARGS__)

  // Additional convenience global const(s)
  DISPLAYCONFIG_PATH_INFO AVAILABLE_AND_ACTIVE_PATH {
    []() {
      DISPLAYCONFIG_PATH_INFO path;

      path.targetInfo.targetAvailable = TRUE;
      path.flags = DISPLAYCONFIG_PATH_ACTIVE;

      return path;
    }()
  };
  DISPLAYCONFIG_PATH_INFO AVAILABLE_AND_INACTIVE_PATH {
    []() {
      DISPLAYCONFIG_PATH_INFO path;

      path.targetInfo.targetAvailable = TRUE;
      path.flags = ~DISPLAYCONFIG_PATH_ACTIVE;

      return path;
    }()
  };
  std::vector<DISPLAYCONFIG_MODE_INFO> TARGET_AND_SOURCE_MODES {
    []() {
      std::vector<DISPLAYCONFIG_MODE_INFO> modes;

      modes.push_back({});
      modes.back().infoType = DISPLAYCONFIG_MODE_INFO_TYPE_TARGET;
      modes.back().targetMode = {};  // Set the union value

      modes.push_back({});
      modes.back().infoType = DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE;
      modes.back().sourceMode = {};  // Set the union value

      return modes;
    }()
  };
  std::vector<DISPLAYCONFIG_PATH_INFO> PATHS_WITH_SOURCE_IDS {
    []() {
      // Contains the following:
      //    - 2 paths for the same adapter (1 active and 1 inactive)
      //        Note: source ids are out of order which is OK
      //    - 1 active path for a different adapter
      //    - 1 inactive path for yet another different adapter
      std::vector<DISPLAYCONFIG_PATH_INFO> paths;

      paths.push_back(AVAILABLE_AND_ACTIVE_PATH);
      paths.back().sourceInfo.adapterId = { 1, 1 };
      paths.back().sourceInfo.id = 1;

      paths.push_back(AVAILABLE_AND_ACTIVE_PATH);
      paths.back().sourceInfo.adapterId = { 2, 2 };
      paths.back().sourceInfo.id = 0;

      paths.push_back(AVAILABLE_AND_INACTIVE_PATH);
      paths.back().sourceInfo.adapterId = { 1, 1 };
      paths.back().sourceInfo.id = 0;

      paths.push_back(AVAILABLE_AND_INACTIVE_PATH);
      paths.back().sourceInfo.adapterId = { 3, 3 };
      paths.back().sourceInfo.id = 4;

      return paths;
    }()
  };

}  // namespace

namespace display_device {
  // Helper comparison functions
  bool
  operator==(const LUID &lhs, const LUID &rhs) {
    return lhs.HighPart == rhs.HighPart && lhs.LowPart == rhs.LowPart;
  }

  bool
  operator==(const PathSourceIndexData &lhs, const PathSourceIndexData &rhs) {
    return lhs.m_source_id_to_path_index == rhs.m_source_id_to_path_index && lhs.m_adapter_id == rhs.m_adapter_id && lhs.m_active_source == rhs.m_active_source;
  }
}  // namespace display_device

TEST_F_S_MOCKED(IsAvailable) {
  DISPLAYCONFIG_PATH_INFO available_path;
  DISPLAYCONFIG_PATH_INFO unavailable_path;

  available_path.targetInfo.targetAvailable = TRUE;
  unavailable_path.targetInfo.targetAvailable = FALSE;

  EXPECT_EQ(display_device::win_utils::isAvailable(available_path), true);
  EXPECT_EQ(display_device::win_utils::isAvailable(unavailable_path), false);
}

TEST_F_S_MOCKED(IsActiveAndSetActive) {
  DISPLAYCONFIG_PATH_INFO inactive_path;
  DISPLAYCONFIG_PATH_INFO only_active_path;
  DISPLAYCONFIG_PATH_INFO contains_active_path;

  inactive_path.flags = (~DISPLAYCONFIG_PATH_ACTIVE);
  only_active_path.flags = DISPLAYCONFIG_PATH_ACTIVE;
  contains_active_path.flags = inactive_path.flags | only_active_path.flags;

  EXPECT_EQ(display_device::win_utils::isActive(inactive_path), false);
  EXPECT_EQ(display_device::win_utils::isActive(only_active_path), true);
  EXPECT_EQ(display_device::win_utils::isActive(contains_active_path), true);

  display_device::win_utils::setActive(inactive_path);
  display_device::win_utils::setActive(only_active_path);
  display_device::win_utils::setActive(contains_active_path);

  EXPECT_EQ(display_device::win_utils::isActive(inactive_path), true);
  EXPECT_EQ(display_device::win_utils::isActive(only_active_path), true);
  EXPECT_EQ(display_device::win_utils::isActive(contains_active_path), true);
}

TEST_F_S_MOCKED(GetSourceIndex) {
  DISPLAYCONFIG_PATH_INFO path;
  std::vector<DISPLAYCONFIG_MODE_INFO> modes;

  path.sourceInfo.sourceModeInfoIdx = 1;
  modes.push_back({});  // Non-empty list
  modes.push_back({});  // Non-empty list

  EXPECT_EQ(display_device::win_utils::getSourceIndex(path, modes), std::make_optional<UINT32>(1));
}

TEST_F_S_MOCKED(GetSourceIndex, InvalidIndex) {
  DISPLAYCONFIG_PATH_INFO path;
  std::vector<DISPLAYCONFIG_MODE_INFO> modes;

  path.sourceInfo.sourceModeInfoIdx = DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID;
  modes.push_back({});  // Non-empty list

  EXPECT_EQ(display_device::win_utils::getSourceIndex(path, modes), std::nullopt);
}

TEST_F_S_MOCKED(GetSourceIndex, OutOfRangeIndex) {
  DISPLAYCONFIG_PATH_INFO path;
  std::vector<DISPLAYCONFIG_MODE_INFO> modes;

  path.sourceInfo.sourceModeInfoIdx = 1;
  modes.push_back({});  // Non-empty list

  EXPECT_EQ(display_device::win_utils::getSourceIndex(path, modes), std::nullopt);
}

TEST_F_S_MOCKED(GetSourceIndex, EmptyList) {
  DISPLAYCONFIG_PATH_INFO path;
  std::vector<DISPLAYCONFIG_MODE_INFO> modes;

  path.sourceInfo.sourceModeInfoIdx = 0;

  EXPECT_EQ(display_device::win_utils::getSourceIndex(path, modes), std::nullopt);
}

TEST_F_S_MOCKED(SetSourceIndex) {
  DISPLAYCONFIG_PATH_INFO path;
  path.sourceInfo.sourceModeInfoIdx = 123;

  display_device::win_utils::setSourceIndex(path, 1);
  EXPECT_EQ(path.sourceInfo.sourceModeInfoIdx, 1);

  display_device::win_utils::setSourceIndex(path, std::nullopt);
  EXPECT_EQ(path.sourceInfo.sourceModeInfoIdx, DISPLAYCONFIG_PATH_SOURCE_MODE_IDX_INVALID);
}

TEST_F_S_MOCKED(SetTargetIndex) {
  DISPLAYCONFIG_PATH_INFO path;
  path.targetInfo.targetModeInfoIdx = 123;

  display_device::win_utils::setTargetIndex(path, 1);
  EXPECT_EQ(path.targetInfo.targetModeInfoIdx, 1);

  display_device::win_utils::setTargetIndex(path, std::nullopt);
  EXPECT_EQ(path.targetInfo.targetModeInfoIdx, DISPLAYCONFIG_PATH_TARGET_MODE_IDX_INVALID);
}

TEST_F_S_MOCKED(SetDesktopIndex) {
  DISPLAYCONFIG_PATH_INFO path;
  path.targetInfo.desktopModeInfoIdx = 123;

  display_device::win_utils::setDesktopIndex(path, 1);
  EXPECT_EQ(path.targetInfo.desktopModeInfoIdx, 1);

  display_device::win_utils::setDesktopIndex(path, std::nullopt);
  EXPECT_EQ(path.targetInfo.desktopModeInfoIdx, DISPLAYCONFIG_PATH_DESKTOP_IMAGE_IDX_INVALID);
}

TEST_F_S_MOCKED(SetCloneGroupId) {
  DISPLAYCONFIG_PATH_INFO path;
  path.sourceInfo.cloneGroupId = 123;

  display_device::win_utils::setCloneGroupId(path, 1);
  EXPECT_EQ(path.sourceInfo.cloneGroupId, 1);

  display_device::win_utils::setCloneGroupId(path, std::nullopt);
  EXPECT_EQ(path.sourceInfo.cloneGroupId, DISPLAYCONFIG_PATH_CLONE_GROUP_INVALID);
}

TEST_F_S_MOCKED(GetSourceMode) {
  auto *mode { display_device::win_utils::getSourceMode(1, TARGET_AND_SOURCE_MODES) };
  auto *const_mode { display_device::win_utils::getSourceMode(1, const_cast<const std::vector<DISPLAYCONFIG_MODE_INFO> &>(TARGET_AND_SOURCE_MODES)) };

  EXPECT_EQ(mode, const_mode);
  EXPECT_EQ(mode, &TARGET_AND_SOURCE_MODES.at(1).sourceMode);
}

TEST_F_S_MOCKED(GetSourceMode, InvalidIndex) {
  auto *mode { display_device::win_utils::getSourceMode(std::nullopt, TARGET_AND_SOURCE_MODES) };
  auto *const_mode { display_device::win_utils::getSourceMode(std::nullopt, const_cast<const std::vector<DISPLAYCONFIG_MODE_INFO> &>(TARGET_AND_SOURCE_MODES)) };

  EXPECT_EQ(mode, const_mode);
  EXPECT_EQ(mode, nullptr);
}

TEST_F_S_MOCKED(GetSourceMode, OutOfRangeIndex) {
  auto *mode { display_device::win_utils::getSourceMode(2, TARGET_AND_SOURCE_MODES) };
  auto *const_mode { display_device::win_utils::getSourceMode(2, const_cast<const std::vector<DISPLAYCONFIG_MODE_INFO> &>(TARGET_AND_SOURCE_MODES)) };

  EXPECT_EQ(mode, const_mode);
  EXPECT_EQ(mode, nullptr);
}

TEST_F_S_MOCKED(GetSourceMode, InvalidModeType) {
  auto *mode { display_device::win_utils::getSourceMode(0, TARGET_AND_SOURCE_MODES) };
  auto *const_mode { display_device::win_utils::getSourceMode(0, const_cast<const std::vector<DISPLAYCONFIG_MODE_INFO> &>(TARGET_AND_SOURCE_MODES)) };

  EXPECT_EQ(mode, const_mode);
  EXPECT_EQ(mode, nullptr);
}

TEST_F_S_MOCKED(GetSourceMode, EmptyList) {
  std::vector<DISPLAYCONFIG_MODE_INFO> modes;

  auto *mode { display_device::win_utils::getSourceMode(2, modes) };
  auto *const_mode { display_device::win_utils::getSourceMode(2, const_cast<const std::vector<DISPLAYCONFIG_MODE_INFO> &>(modes)) };

  EXPECT_EQ(mode, const_mode);
  EXPECT_EQ(mode, nullptr);
}

TEST_F_S_MOCKED(GetDeviceInfo, AvailablePath, ActivePath, MustBeActiveIsTrue) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return("DevicePath"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return("DeviceId"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(1)
    .WillOnce(Return("DisplayName"));

  const auto result { display_device::win_utils::getDeviceInfoForValidPath(m_layer, AVAILABLE_AND_ACTIVE_PATH, true) };
  ASSERT_TRUE(result);

  EXPECT_EQ(result->m_device_path, "DevicePath");
  EXPECT_EQ(result->m_device_id, "DeviceId");
}

TEST_F_S_MOCKED(GetDeviceInfo, AvailablePath, ActivePath, MustBeActiveIsFalse) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return("DevicePath"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return("DeviceId"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(1)
    .WillOnce(Return("DisplayName"));

  const auto result { display_device::win_utils::getDeviceInfoForValidPath(m_layer, AVAILABLE_AND_ACTIVE_PATH, false) };
  ASSERT_TRUE(result);

  EXPECT_EQ(result->m_device_path, "DevicePath");
  EXPECT_EQ(result->m_device_id, "DeviceId");
}

TEST_F_S_MOCKED(GetDeviceInfo, AvailablePath, InactivePath, MustBeActiveIsTrue) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return("DevicePath"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return("DeviceId"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(1)
    .WillOnce(Return(""));

  EXPECT_EQ(display_device::win_utils::getDeviceInfoForValidPath(m_layer, AVAILABLE_AND_ACTIVE_PATH, true), std::nullopt);
}

TEST_F_S_MOCKED(GetDeviceInfo, AvailablePath, InactivePath, MustBeActiveIsFalse) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return("DevicePath"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return("DeviceId"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(1)
    .WillOnce(Return("DisplayName"));

  const auto result { display_device::win_utils::getDeviceInfoForValidPath(m_layer, AVAILABLE_AND_INACTIVE_PATH, false) };
  ASSERT_TRUE(result);

  EXPECT_EQ(result->m_device_path, "DevicePath");
  EXPECT_EQ(result->m_device_id, "DeviceId");
}

TEST_F_S_MOCKED(GetDeviceInfo, AvailablePath, ActivePath, EmptyDeviceId) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return("DevicePath"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return(""));

  EXPECT_EQ(display_device::win_utils::getDeviceInfoForValidPath(m_layer, AVAILABLE_AND_ACTIVE_PATH, true), std::nullopt);
}

TEST_F_S_MOCKED(GetDeviceInfo, AvailablePath, ActivePath, EmptyDevicePath) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return(""));

  EXPECT_EQ(display_device::win_utils::getDeviceInfoForValidPath(m_layer, AVAILABLE_AND_ACTIVE_PATH, true), std::nullopt);
}

TEST_F_S_MOCKED(GetDeviceInfo, AvailablePath, InactivePath) {
  EXPECT_EQ(display_device::win_utils::getDeviceInfoForValidPath(m_layer, AVAILABLE_AND_INACTIVE_PATH, true), std::nullopt);
}

TEST_F_S_MOCKED(GetDeviceInfo, UnavailablePath, ActivePath) {
  DISPLAYCONFIG_PATH_INFO path;

  path.targetInfo.targetAvailable = FALSE;
  path.flags = DISPLAYCONFIG_PATH_ACTIVE;

  EXPECT_EQ(display_device::win_utils::getDeviceInfoForValidPath(m_layer, path, false), std::nullopt);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(4)
    .WillOnce(Return("Path1"))
    .WillOnce(Return("Path2"))
    .WillOnce(Return("Path1"))
    .WillOnce(Return("Path3"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(4)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(4)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId3"));

  const display_device::PathSourceIndexDataMap expected_data {
    { "DeviceId1", { { { 0, 2 }, { 1, 0 } }, { 1, 1 }, { 1 } } },
    { "DeviceId2", { { { 0, 1 } }, { 2, 2 }, { 0 } } },
    { "DeviceId3", { { { 4, 3 } }, { 3, 3 }, std::nullopt } }
  };
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, PATHS_WITH_SOURCE_IDS), expected_data);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths, TransientPathIssues) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(4)
    .WillOnce(Return("Path1"))
    .WillOnce(Return(""))  // Path is not available for some reason
    .WillOnce(Return("Path1"))
    .WillOnce(Return("Path3"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(3)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(3)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId3"));

  const display_device::PathSourceIndexDataMap expected_data {
    { "DeviceId1", { { { 0, 2 }, { 1, 0 } }, { 1, 1 }, { 1 } } },
    { "DeviceId3", { { { 4, 3 } }, { 3, 3 }, std::nullopt } }
  };
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, PATHS_WITH_SOURCE_IDS), expected_data);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths, DuplicatePathsWithDifferentIds) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(2)
    .WillRepeatedly(Return("PathSame"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(2)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(2)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"));

  const display_device::PathSourceIndexDataMap expected_data {};
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, PATHS_WITH_SOURCE_IDS), expected_data);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths, DifferentPathsWithSameId) {
  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(3)
    .WillOnce(Return("Path1"))
    .WillOnce(Return("Path2"))
    .WillOnce(Return("Path3"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(3)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(3)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId1"));

  const display_device::PathSourceIndexDataMap expected_data {};
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, PATHS_WITH_SOURCE_IDS), expected_data);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths, MismatchingAdapterIdsForPaths) {
  std::vector<DISPLAYCONFIG_PATH_INFO> paths { PATHS_WITH_SOURCE_IDS };

  // Invalidate the adapter id
  paths.at(2).sourceInfo.adapterId.HighPart++;

  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(3)
    .WillOnce(Return("Path1"))
    .WillOnce(Return("Path2"))
    .WillOnce(Return("Path1"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(3)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(3)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId1"));

  const display_device::PathSourceIndexDataMap expected_data {};
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, paths), expected_data);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths, ActiveDeviceNotFirst) {
  std::vector<DISPLAYCONFIG_PATH_INFO> paths { PATHS_WITH_SOURCE_IDS };

  // Swapping around the active/inactive pair
  std::swap(paths.at(0), paths.at(2));

  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(3)
    .WillOnce(Return("Path1"))
    .WillOnce(Return("Path2"))
    .WillOnce(Return("Path1"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(3)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(3)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId1"));

  const display_device::PathSourceIndexDataMap expected_data {};
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, paths), expected_data);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths, DuplicateSourceIds) {
  std::vector<DISPLAYCONFIG_PATH_INFO> paths { PATHS_WITH_SOURCE_IDS };

  // Making sure source ids match (adapter ids don't matter)
  paths.at(0).sourceInfo.id = paths.at(2).sourceInfo.id;

  EXPECT_CALL(m_layer, getMonitorDevicePath(_))
    .Times(3)
    .WillOnce(Return("Path1"))
    .WillOnce(Return("Path2"))
    .WillOnce(Return("Path1"));
  EXPECT_CALL(m_layer, getDisplayName(_))
    .Times(3)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(m_layer, getDeviceId(_))
    .Times(3)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId1"));

  const display_device::PathSourceIndexDataMap expected_data {};
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, paths), expected_data);
}

TEST_F_S_MOCKED(CollectSourceDataForMatchingPaths, EmptyList) {
  std::vector<DISPLAYCONFIG_PATH_INFO> paths;

  const display_device::PathSourceIndexDataMap expected_data {};
  EXPECT_EQ(display_device::win_utils::collectSourceDataForMatchingPaths(m_layer, paths), expected_data);
}
