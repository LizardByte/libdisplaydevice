// local includes
#include "displaydevice/windows/winutils.h"
#include "fixtures.h"
#include "mocks/mockwinapilayer.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class WinUtilsMocked: public BaseTest {
  public:
    StrictMock<display_device::MockWinApiLayer> m_layer;
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, WinUtilsMocked, __VA_ARGS__)

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
}  // namespace

TEST_F_S_MOCKED(IsAvailable) {
  DISPLAYCONFIG_PATH_INFO available_path;
  DISPLAYCONFIG_PATH_INFO unavailable_path;

  available_path.targetInfo.targetAvailable = TRUE;
  unavailable_path.targetInfo.targetAvailable = FALSE;

  EXPECT_EQ(display_device::win_utils::isAvailable(available_path), true);
  EXPECT_EQ(display_device::win_utils::isAvailable(unavailable_path), false);
}

TEST_F_S_MOCKED(IsActive) {
  DISPLAYCONFIG_PATH_INFO inactive_path;
  DISPLAYCONFIG_PATH_INFO only_active_path;
  DISPLAYCONFIG_PATH_INFO contains_active_path;

  inactive_path.flags = (~DISPLAYCONFIG_PATH_ACTIVE);
  only_active_path.flags = DISPLAYCONFIG_PATH_ACTIVE;
  contains_active_path.flags = inactive_path.flags | only_active_path.flags;

  EXPECT_EQ(display_device::win_utils::isActive(inactive_path), false);
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
