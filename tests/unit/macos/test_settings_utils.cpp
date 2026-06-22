// local includes
#include "display_device/macos/settings_utils.h"
#include "fixtures/fixtures.h"
#include "utils/mock_mac_display_device.h"

namespace {
  using ::testing::Return;
  using ::testing::StrictMock;
}  // namespace

// Specialized TEST macro(s) for this test file
#define TEST_S(...) DD_MAKE_TEST(TEST, MacSettingsUtils, __VA_ARGS__)

TEST_S(FlattenTopology) {
  EXPECT_EQ(display_device::mac_utils::flattenTopology({}), display_device::StringSet {});
  EXPECT_EQ(
    display_device::mac_utils::flattenTopology({{"DeviceId1"}, {"DeviceId2", "DeviceId3"}, {"DeviceId1"}}),
    (display_device::StringSet {"DeviceId1", "DeviceId2", "DeviceId3"})
  );
}

TEST_S(GetPrimaryDevice) {
  StrictMock<display_device::MockMacDisplayDevice> mac_dd;

  EXPECT_CALL(mac_dd, isPrimary("DeviceId1"))
    .Times(1)
    .WillOnce(Return(false));
  EXPECT_CALL(mac_dd, isPrimary("DeviceId2"))
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_EQ(display_device::mac_utils::getPrimaryDevice(mac_dd, {{"DeviceId1"}, {"DeviceId2"}}), "DeviceId2");
}

TEST_S(ComputeInitialState) {
  const display_device::MacSingleDisplayConfigState::Initial previous_state {
    {{"DeviceId3"}},
    {"DeviceId3"}
  };
  const display_device::EnumeratedDeviceList devices {
    {.m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info {.m_primary = true}},
    {.m_device_id = "DeviceId2", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
  };

  EXPECT_EQ(display_device::mac_utils::computeInitialState(previous_state, {{"DeviceId1"}}, devices), previous_state);
  EXPECT_EQ(
    display_device::mac_utils::computeInitialState(std::nullopt, {{"DeviceId1"}, {"DeviceId2"}}, devices),
    (display_device::MacSingleDisplayConfigState::Initial {{{"DeviceId1"}, {"DeviceId2"}}, {"DeviceId1"}})
  );
}

TEST_S(ComputeInitialState, NoPrimaryDevice) {
  const display_device::EnumeratedDeviceList devices {
    {.m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
    {.m_device_id = "DeviceId2"}
  };

  EXPECT_FALSE(display_device::mac_utils::computeInitialState(std::nullopt, {{"DeviceId1"}}, devices));
}

TEST_S(StripInitialState) {
  const display_device::MacSingleDisplayConfigState::Initial initial_state {
    {{"DeviceId1", "DeviceId2"}, {"DeviceId3"}},
    {"DeviceId2", "DeviceId4"}
  };
  const display_device::EnumeratedDeviceList devices {
    {.m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
    {.m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info {.m_primary = true}},
  };

  EXPECT_EQ(
    display_device::mac_utils::stripInitialState(initial_state, devices),
    (display_device::MacSingleDisplayConfigState::Initial {{{"DeviceId1"}, {"DeviceId3"}}, {"DeviceId3"}})
  );
}

TEST_S(ComputeNewDisplayModes) {
  const display_device::MacDeviceDisplayModeMap original_modes {
    {"DeviceId1", {{1920, 1080}, {60, 1}}},
    {"DeviceId2", {{1920, 1080}, {120, 1}}},
    {"DeviceId3", {{2560, 1440}, {60, 1}}},
  };

  EXPECT_EQ(
    display_device::mac_utils::computeNewDisplayModes(
      display_device::Resolution {1280, 720},
      display_device::FloatingPoint {display_device::Rational {144, 1}},
      false,
      "DeviceId1",
      {"DeviceId2"},
      original_modes
    ),
    (display_device::MacDeviceDisplayModeMap {
      {"DeviceId1", {{1280, 720}, {144, 1}}},
      {"DeviceId2", {{1280, 720}, {120, 1}}},
      {"DeviceId3", {{2560, 1440}, {60, 1}}},
    })
  );

  EXPECT_EQ(
    display_device::mac_utils::computeNewDisplayModes(
      std::nullopt,
      display_device::FloatingPoint {119.88},
      true,
      "DeviceId1",
      {"DeviceId2"},
      original_modes
    ),
    (display_device::MacDeviceDisplayModeMap {
      {"DeviceId1", {{1920, 1080}, {1198800, 10000}}},
      {"DeviceId2", {{1920, 1080}, {1198800, 10000}}},
      {"DeviceId3", {{2560, 1440}, {60, 1}}},
    })
  );
}

TEST_S(NoopGuard) {
  EXPECT_NO_THROW(display_device::mac_utils::noopGuard());
}
