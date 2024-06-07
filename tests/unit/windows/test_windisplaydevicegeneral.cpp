// local includes
#include "displaydevice/windows/winapilayer.h"
#include "displaydevice/windows/winapiutils.h"
#include "displaydevice/windows/windisplaydevice.h"
#include "fixtures.h"
#include "utils/comparison.h"
#include "utils/mockwinapilayer.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::HasSubstr;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class WinDisplayDeviceGeneral: public BaseTest {
  public:
    std::shared_ptr<display_device::WinApiLayer> m_layer { std::make_shared<display_device::WinApiLayer>() };
    display_device::WinDisplayDevice m_win_dd { m_layer };
  };

  class WinDisplayDeviceGeneralMocked: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockWinApiLayer>> m_layer { std::make_shared<StrictMock<display_device::MockWinApiLayer>>() };
    display_device::WinDisplayDevice m_win_dd { m_layer };
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, WinDisplayDeviceGeneral, __VA_ARGS__)
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, WinDisplayDeviceGeneralMocked, __VA_ARGS__)

  // Additional convenience global const(s)
  const UINT32 FLAGS { SDC_VALIDATE | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_VIRTUAL_MODE_AWARE };
}  // namespace

TEST_F_S(NullptrLayerProvided) {
  EXPECT_THAT([]() { const auto win_dd { display_device::WinDisplayDevice { nullptr } }; },
    ThrowsMessage<std::logic_error>(HasSubstr("Nullptr provided for WinApiLayerInterface in WinDisplayDevice!")));
}

TEST_F_S(IsApiAccessAvailable) {
  // There is no real way to make it return false without logging out or do something complicated :/
  EXPECT_TRUE(m_win_dd.isApiAccessAvailable());
}

TEST_F_S_MOCKED(IsApiAccessAvailable) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::All))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE));
  EXPECT_CALL(*m_layer, setDisplayConfig(ut_consts::PAM_3_ACTIVE->m_paths, ut_consts::PAM_3_ACTIVE->m_modes, FLAGS))
    .Times(1)
    .WillOnce(Return(ERROR_SUCCESS));
  EXPECT_CALL(*m_layer, getErrorString(ERROR_SUCCESS))
    .Times(1)
    .WillRepeatedly(Return("ErrorDesc"));

  EXPECT_TRUE(m_win_dd.isApiAccessAvailable());
}

TEST_F_S_MOCKED(IsApiAccessAvailable, FailedToGetDisplayData) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::All))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_NULL));

  EXPECT_FALSE(m_win_dd.isApiAccessAvailable());
}

TEST_F_S_MOCKED(IsApiAccessAvailable, FailedToSetDisplayConfig) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::All))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE));
  EXPECT_CALL(*m_layer, setDisplayConfig(ut_consts::PAM_3_ACTIVE->m_paths, ut_consts::PAM_3_ACTIVE->m_modes, FLAGS))
    .Times(1)
    .WillOnce(Return(ERROR_ACCESS_DENIED));
  EXPECT_CALL(*m_layer, getErrorString(ERROR_ACCESS_DENIED))
    .Times(1)
    .WillRepeatedly(Return("ErrorDesc"));

  EXPECT_FALSE(m_win_dd.isApiAccessAvailable());
}

TEST_F_S(GetDisplayName) {
  const auto all_devices { m_layer->queryDisplayConfig(display_device::QueryType::All) };
  ASSERT_TRUE(all_devices);

  for (const auto &path : all_devices->m_paths) {
    const auto device_id { m_layer->getDeviceId(path) };
    const auto display_name { m_win_dd.getDisplayName(device_id) };
    const auto active_path { display_device::win_utils::getActivePath(*m_layer, device_id, all_devices->m_paths) };

    if (&path == active_path) {
      EXPECT_EQ(display_name, m_layer->getDisplayName(path));
    }
    else {
      EXPECT_EQ(display_name, active_path ? m_layer->getDisplayName(*active_path) : std::string {});
    }
  }
}

TEST_F_S_MOCKED(GetDisplayName) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE));
  EXPECT_CALL(*m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return("PathX"));
  EXPECT_CALL(*m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getDisplayName(_))
    .Times(2)
    .WillOnce(Return("DisplayName1"))
    .WillOnce(Return("DisplayName1"));

  EXPECT_EQ(m_win_dd.getDisplayName("DeviceId1"), "DisplayName1");
}

TEST_F_S_MOCKED(GetDisplayName, EmptyDeviceId) {
  EXPECT_EQ(m_win_dd.getDisplayName(""), std::string {});
}

TEST_F_S_MOCKED(GetDisplayName, FailedToGetDisplayData) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_NULL));

  EXPECT_EQ(m_win_dd.getDisplayName("DeviceId1"), std::string {});
}

TEST_F_S_MOCKED(GetDisplayName, FailedToGetActivePath) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_EMPTY));

  EXPECT_EQ(m_win_dd.getDisplayName("DeviceId1"), std::string {});
}

TEST_F_S_MOCKED(GetDisplayName, EmptyDisplayName) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE));
  EXPECT_CALL(*m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillOnce(Return("PathX"));
  EXPECT_CALL(*m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getDisplayName(_))
    .Times(2)
    .WillOnce(Return("DisplayName1"))
    .WillOnce(Return(""));

  EXPECT_EQ(m_win_dd.getDisplayName("DeviceId1"), std::string {});
}
