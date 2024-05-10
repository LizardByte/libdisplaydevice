// local includes
#include "displaydevice/windows/winapilayer.h"
#include "displaydevice/windows/windisplaydevice.h"
#include "fixtures.h"
#include "mocks/mockwinapilayer.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class WinDisplayDeviceTopology: public BaseTest {
  public:
    std::shared_ptr<display_device::WinApiLayer> m_layer { std::make_shared<display_device::WinApiLayer>() };
    display_device::WinDisplayDevice m_win_dd { m_layer };
  };

  class WinDisplayDeviceTopologyMocked: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockWinApiLayer>> m_layer { std::make_shared<StrictMock<display_device::MockWinApiLayer>>() };
    display_device::WinDisplayDevice m_win_dd { m_layer };
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, WinDisplayDeviceTopology, __VA_ARGS__)
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, WinDisplayDeviceTopologyMocked, __VA_ARGS__)
}  // namespace

TEST_F_S(GetCurrentTopology, FromSystem) {
  const auto active_devices { m_layer->queryDisplayConfig(display_device::QueryType::Active) };
  ASSERT_TRUE(active_devices);

  if (active_devices->m_paths.empty()) {
    GTEST_SKIP_("No active devices are available in the system.");
  }

  std::set<std::string> expected_devices;
  for (const auto &path : active_devices->m_paths) {
    const auto device_id { m_layer->getDeviceId(path) };
    EXPECT_FALSE(device_id.empty());
    EXPECT_TRUE(expected_devices.insert(device_id).second);
  }

  // It is enough to check whether the topology contains expected ids - others test cases check the structure.
  std::set<std::string> flattened_topology;
  const auto current_topology { m_win_dd.getCurrentTopology() };
  for (const auto &group : current_topology) {
    for (const auto &device_id : group) {
      EXPECT_FALSE(device_id.empty());
      EXPECT_TRUE(flattened_topology.insert(device_id).second);
    }
  }

  EXPECT_EQ(flattened_topology, expected_devices);
}

TEST_F_S_MOCKED(GetCurrentTopology, ExtendedDisplaysOnly) {
  EXPECT_CALL(*m_layer, getMonitorDevicePath(_))
    .Times(3)
    .WillRepeatedly(Return("PathX"));
  EXPECT_CALL(*m_layer, getDisplayName(_))
    .Times(3)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(*m_layer, getDeviceId(_))
    .Times(3)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId3"));
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE));

  const display_device::ActiveTopology expected_topology { { "DeviceId1" }, { "DeviceId2" }, { "DeviceId3" } };
  EXPECT_EQ(m_win_dd.getCurrentTopology(), expected_topology);
}

TEST_F_S_MOCKED(GetCurrentTopology, ExtendedAndDuplicatedDisplays) {
  EXPECT_CALL(*m_layer, getMonitorDevicePath(_))
    .Times(4)
    .WillRepeatedly(Return("PathX"));
  EXPECT_CALL(*m_layer, getDisplayName(_))
    .Times(4)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(*m_layer, getDeviceId(_))
    .Times(4)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId3"))
    .WillOnce(Return("DeviceId4"));
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_4_ACTIVE_WITH_2_DUPLICATES));

  const display_device::ActiveTopology expected_topology { { "DeviceId1" }, { "DeviceId2", "DeviceId3" }, { "DeviceId4" } };
  EXPECT_EQ(m_win_dd.getCurrentTopology(), expected_topology);
}

TEST_F_S_MOCKED(GetCurrentTopology, PathsContainingInvalidIndexes) {
  EXPECT_CALL(*m_layer, getMonitorDevicePath(_))
    .Times(1)
    .WillRepeatedly(Return("PathX"));
  EXPECT_CALL(*m_layer, getDisplayName(_))
    .Times(1)
    .WillRepeatedly(Return("DisplayNameX"));
  EXPECT_CALL(*m_layer, getDeviceId(_))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE_WITH_INVALID_MODE_IDX));

  const display_device::ActiveTopology expected_topology {};
  EXPECT_EQ(m_win_dd.getCurrentTopology(), expected_topology);
}

TEST_F_S_MOCKED(GetCurrentTopology, TransientDisplayIssues) {
  EXPECT_CALL(*m_layer, getMonitorDevicePath(_))
    .Times(3)
    .WillRepeatedly(Return("PathX"));
  EXPECT_CALL(*m_layer, getDisplayName(_))
    .Times(3)
    .WillOnce(Return("DisplayName1"))
    .WillOnce(Return(""))  // The display name does not exist for some reason, but it was still reported as active
    .WillOnce(Return("DisplayName2"));
  EXPECT_CALL(*m_layer, getDeviceId(_))
    .Times(3)
    .WillOnce(Return("DeviceId1"))
    .WillOnce(Return("DeviceId2"))
    .WillOnce(Return("DeviceId3"));
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE));

  const display_device::ActiveTopology expected_topology { { "DeviceId1" }, { "DeviceId3" } };
  EXPECT_EQ(m_win_dd.getCurrentTopology(), expected_topology);
}

TEST_F_S_MOCKED(GetCurrentTopology, EmptyDeviceList) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(_))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_EMPTY));

  const display_device::ActiveTopology expected_topology {};
  EXPECT_EQ(m_win_dd.getCurrentTopology(), expected_topology);
}

TEST_F_S_MOCKED(GetCurrentTopology, NullDeviceList) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(_))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_NULL));

  const display_device::ActiveTopology expected_topology {};
  EXPECT_EQ(m_win_dd.getCurrentTopology(), expected_topology);
}

TEST_F_S_MOCKED(IsTopologyValid) {
  EXPECT_EQ(m_win_dd.isTopologyValid({ /* no groups */ }), false);  // TODO: come back to decide whether this can be valid once `set_topology` is implemented
  EXPECT_EQ(m_win_dd.isTopologyValid({ { /* empty group */ } }), false);  // TODO: come back to decide whether this can be valid once `set_topology` is implemented
  EXPECT_EQ(m_win_dd.isTopologyValid({ { "ID_1" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyValid({ { "ID_1" }, { "ID_2" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyValid({ { "ID_1", "ID_2" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyValid({ { "ID_1", "ID_1" } }), false);
  EXPECT_EQ(m_win_dd.isTopologyValid({ { "ID_1" }, { "ID_1" } }), false);
  EXPECT_EQ(m_win_dd.isTopologyValid({ { "ID_1", "ID_2", "ID_3" } }), false);
  EXPECT_EQ(m_win_dd.isTopologyValid({ { "ID_1" }, { "ID_2" }, { "ID_3" }, { "ID_4" }, { "ID_5" } }), true);
}

TEST_F_S_MOCKED(isTopologyTheSame) {
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ /* no groups */ }, { /* no groups */ }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { /* empty group */ } }, { { /* empty group */ } }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { /* empty group */ } }, { { /* empty group */ }, { /* empty group */ } }), false);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1" } }, { { "ID_1" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1" } }, { { "ID_1" }, { "ID_2" } }), false);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1" }, { "ID_2" } }, { { "ID_1" }, { "ID_2" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1" }, { "ID_2" } }, { { "ID_2" }, { "ID_1" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1" }, { "ID_2" } }, { { "ID_1", "ID_2" } }), false);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1", "ID_2" } }, { { "ID_1", "ID_2" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1", "ID_2" } }, { { "ID_2", "ID_1" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1", "ID_2" } }, { { "ID_2", "ID_1" }, { "ID_3" } }), false);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_1", "ID_2" }, { "ID_3" } }, { { "ID_2", "ID_1" }, { "ID_3" } }), true);
  EXPECT_EQ(m_win_dd.isTopologyTheSame({ { "ID_3" }, { "ID_1", "ID_2" } }, { { "ID_2", "ID_1" }, { "ID_3" } }), true);
}
