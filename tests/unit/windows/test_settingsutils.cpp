// local includes
#include "displaydevice/windows/settingsutils.h"
#include "fixtures/fixtures.h"
#include "utils/comparison.h"
#include "utils/mockwindisplaydevice.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class SettingsUtilsMocked: public BaseTest {
  public:
    StrictMock<display_device::MockWinDisplayDevice> m_dd_api {};
  };

// Specialized TEST macro(s) for this test SettingsManagerGeneralMocked
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, SettingsUtilsMocked, __VA_ARGS__)

  // Additional convenience global const(s)
  const display_device::ActiveTopology DEFAULT_INITIAL_TOPOLOGY { { "DeviceId1", "DeviceId2" }, { "DeviceId3" } };
  const display_device::DeviceDisplayModeMap DEFAULT_CURRENT_MODES {
    { "DeviceId1", { { 1080, 720 }, { 120, 1 } } },
    { "DeviceId2", { { 1920, 1080 }, { 60, 1 } } },
    { "DeviceId3", { { 2560, 1440 }, { 30, 1 } } }
  };
}  // namespace

TEST_F_S_MOCKED(FlattenTopology) {
  EXPECT_EQ(display_device::win_utils::flattenTopology({ { "DeviceId1" }, { "DeviceId2", "DeviceId3" }, {}, { "DeviceId2" } }), (std::set<std::string> { "DeviceId1", "DeviceId2", "DeviceId3" }));
  EXPECT_EQ(display_device::win_utils::flattenTopology({ {}, {}, {} }), std::set<std::string> {});
  EXPECT_EQ(display_device::win_utils::flattenTopology({}), std::set<std::string> {});
}

TEST_F_S_MOCKED(ComputeInitialState, PreviousStateIsUsed) {
  const display_device::SingleDisplayConfigState::Initial prev_state { DEFAULT_INITIAL_TOPOLOGY };
  EXPECT_EQ(display_device::win_utils::computeInitialState(prev_state, {}, {}), std::make_optional(prev_state));
}

TEST_F_S_MOCKED(ComputeInitialState, NewStateIsComputed) {
  const display_device::ActiveTopology topology_before_changes { DEFAULT_INITIAL_TOPOLOGY };
  const display_device::EnumeratedDeviceList devices {
    { .m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } },
    { .m_device_id = "DeviceId2", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } },
    { .m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info { .m_primary = false } },
    { .m_device_id = "DeviceId4" }
  };

  EXPECT_EQ(display_device::win_utils::computeInitialState(std::nullopt, topology_before_changes, devices),
    (display_device::SingleDisplayConfigState::Initial { topology_before_changes, { "DeviceId1", "DeviceId2" } }));
}

TEST_F_S_MOCKED(ComputeInitialState, NoPrimaryDevices) {
  EXPECT_EQ(display_device::win_utils::computeInitialState(std::nullopt, { { "DeviceId1", "DeviceId2" } }, {}), std::nullopt);
}

TEST_F_S_MOCKED(ComputeNewTopology, VerifyOnly) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  EXPECT_EQ(display_device::win_utils::computeNewTopology(DevicePrep::VerifyOnly, false, "DeviceId4", { "DeviceId5", "DeviceId6" }, DEFAULT_INITIAL_TOPOLOGY), DEFAULT_INITIAL_TOPOLOGY);
}

TEST_F_S_MOCKED(ComputeNewTopology, EnsureOnlyDisplay) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  EXPECT_EQ(display_device::win_utils::computeNewTopology(DevicePrep::EnsureOnlyDisplay, true, "DeviceId4", { "DeviceId5", "DeviceId6" }, DEFAULT_INITIAL_TOPOLOGY),
    (display_device::ActiveTopology { { "DeviceId4", "DeviceId5", "DeviceId6" } }));
  EXPECT_EQ(display_device::win_utils::computeNewTopology(DevicePrep::EnsureOnlyDisplay, false, "DeviceId4", { "DeviceId5", "DeviceId6" }, DEFAULT_INITIAL_TOPOLOGY),
    display_device::ActiveTopology { { "DeviceId4" } });
}

TEST_F_S_MOCKED(ComputeNewTopology, EnsureActive) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  EXPECT_EQ(display_device::win_utils::computeNewTopology(DevicePrep::EnsureActive, true, "DeviceId4", { "DeviceId5", "DeviceId6" }, { { "DeviceId4" } }),
    display_device::ActiveTopology { { "DeviceId4" } });
  EXPECT_EQ(display_device::win_utils::computeNewTopology(DevicePrep::EnsureActive, true, "DeviceId4", { "DeviceId5", "DeviceId6" }, { { "DeviceId3" } }),
    (display_device::ActiveTopology { { "DeviceId3" }, { "DeviceId4" } }));
}

TEST_F_S_MOCKED(ComputeNewTopology, EnsurePrimary) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  EXPECT_EQ(display_device::win_utils::computeNewTopology(DevicePrep::EnsurePrimary, true, "DeviceId4", { "DeviceId5", "DeviceId6" }, { { "DeviceId4" } }),
    display_device::ActiveTopology { { "DeviceId4" } });
  EXPECT_EQ(display_device::win_utils::computeNewTopology(DevicePrep::EnsurePrimary, true, "DeviceId4", { "DeviceId5", "DeviceId6" }, { { "DeviceId3" } }),
    (display_device::ActiveTopology { { "DeviceId3" }, { "DeviceId4" } }));
}

TEST_F_S_MOCKED(ComputeNewDisplayModes, PrimaryDevices) {
  auto expected_modes { DEFAULT_CURRENT_MODES };
  expected_modes["DeviceId1"] = { { 1920, 1080 }, { 1200000, 10000 } };
  expected_modes["DeviceId2"] = { { 1920, 1080 }, { 1200000, 10000 } };

  EXPECT_EQ(display_device::win_utils::computeNewDisplayModes({ { 1920, 1080 } }, { 120. }, true, "DeviceId1", { "DeviceId2" }, DEFAULT_CURRENT_MODES), expected_modes);
}

TEST_F_S_MOCKED(ComputeNewDisplayModes, NonPrimaryDevices) {
  auto expected_modes { DEFAULT_CURRENT_MODES };
  expected_modes["DeviceId1"] = { { 1920, 1080 }, { 1200000, 10000 } };
  expected_modes["DeviceId2"] = { { 1920, 1080 }, expected_modes["DeviceId2"].m_refresh_rate };

  EXPECT_EQ(display_device::win_utils::computeNewDisplayModes({ { 1920, 1080 } }, { 120. }, false, "DeviceId1", { "DeviceId2" }, DEFAULT_CURRENT_MODES), expected_modes);
}

TEST_F_S_MOCKED(StripInitialState, NoStripIsPerformed) {
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };
  const display_device::EnumeratedDeviceList devices {
    { .m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } },
    { .m_device_id = "DeviceId2", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } },
    { .m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info { .m_primary = false } },
    { .m_device_id = "DeviceId4" }
  };

  EXPECT_EQ(display_device::win_utils::stripInitialState(initial_state, devices), initial_state);
}

TEST_F_S_MOCKED(StripInitialState, AllDevicesAreStripped) {
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };
  const display_device::EnumeratedDeviceList devices {
    { .m_device_id = "DeviceId4" }
  };

  EXPECT_EQ(display_device::win_utils::stripInitialState(initial_state, devices), std::nullopt);
}

TEST_F_S_MOCKED(StripInitialState, OneNonPrimaryDeviceStripped) {
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };
  const display_device::EnumeratedDeviceList devices {
    { .m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } },
    { .m_device_id = "DeviceId2", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } }
  };

  EXPECT_EQ(display_device::win_utils::stripInitialState(initial_state, devices), (display_device::SingleDisplayConfigState::Initial { { { "DeviceId1", "DeviceId2" } }, { "DeviceId1", "DeviceId2" } }));
}

TEST_F_S_MOCKED(StripInitialState, OnePrimaryDeviceStripped) {
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };
  const display_device::EnumeratedDeviceList devices {
    { .m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info { .m_primary = false } },
    { .m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } },
  };

  EXPECT_EQ(display_device::win_utils::stripInitialState(initial_state, devices), (display_device::SingleDisplayConfigState::Initial { { { "DeviceId1" }, { "DeviceId3" } }, { "DeviceId1" } }));
}

TEST_F_S_MOCKED(StripInitialState, PrimaryDevicesCompletelyStripped) {
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };
  const display_device::EnumeratedDeviceList devices {
    { .m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info { .m_primary = false } }
  };

  EXPECT_EQ(display_device::win_utils::stripInitialState(initial_state, devices), std::nullopt);
}

TEST_F_S_MOCKED(StripInitialState, PrimaryDevicesCompletelyReplaced) {
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };
  const display_device::EnumeratedDeviceList devices {
    { .m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info { .m_primary = true } }
  };

  EXPECT_EQ(display_device::win_utils::stripInitialState(initial_state, devices), (display_device::SingleDisplayConfigState::Initial { { { "DeviceId3" } }, { "DeviceId3" } }));
}

TEST_F_S_MOCKED(ComputeNewTopologyAndMetadata, EmptyDeviceId, AdditionalDevicesNotStripped) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const std::string device_id {};
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };

  const auto &[new_topology, device_to_configure, additional_devices_to_configure] =
    display_device::win_utils::computeNewTopologyAndMetadata(DevicePrep::EnsureActive, device_id, initial_state);
  EXPECT_EQ(new_topology, DEFAULT_INITIAL_TOPOLOGY);
  EXPECT_EQ(device_to_configure, "DeviceId1");
  EXPECT_EQ(additional_devices_to_configure, std::set<std::string> { "DeviceId2" });
}

TEST_F_S_MOCKED(ComputeNewTopologyAndMetadata, EmptyDeviceId, AdditionalDevicesStripped) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const std::string device_id {};
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId3", "DeviceId4" } };

  const auto &[new_topology, device_to_configure, additional_devices_to_configure] =
    display_device::win_utils::computeNewTopologyAndMetadata(DevicePrep::EnsureActive, device_id, initial_state);
  EXPECT_EQ(new_topology, DEFAULT_INITIAL_TOPOLOGY);
  EXPECT_EQ(device_to_configure, "DeviceId3");
  EXPECT_EQ(additional_devices_to_configure, std::set<std::string> {});
}

TEST_F_S_MOCKED(ComputeNewTopologyAndMetadata, ValidDeviceId, WithAdditionalDevices) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const std::string device_id { "DeviceId1" };
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };

  const auto &[new_topology, device_to_configure, additional_devices_to_configure] =
    display_device::win_utils::computeNewTopologyAndMetadata(DevicePrep::EnsureActive, device_id, initial_state);
  EXPECT_EQ(new_topology, DEFAULT_INITIAL_TOPOLOGY);
  EXPECT_EQ(device_to_configure, device_id);
  EXPECT_EQ(additional_devices_to_configure, std::set<std::string> { "DeviceId2" });
}

TEST_F_S_MOCKED(ComputeNewTopologyAndMetadata, ValidDeviceId, NoAdditionalDevices) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const std::string device_id { "DeviceId1" };
  const display_device::SingleDisplayConfigState::Initial initial_state { DEFAULT_INITIAL_TOPOLOGY, { "DeviceId1", "DeviceId2" } };

  const auto &[new_topology, device_to_configure, additional_devices_to_configure] =
    display_device::win_utils::computeNewTopologyAndMetadata(DevicePrep::EnsureOnlyDisplay, device_id, initial_state);
  EXPECT_EQ(new_topology, display_device::ActiveTopology { { "DeviceId1" } });
  EXPECT_EQ(device_to_configure, device_id);
  EXPECT_EQ(additional_devices_to_configure, std::set<std::string> {});
}

TEST_F_S_MOCKED(TopologyGuardFn, Success) {
  EXPECT_CALL(m_dd_api, setTopology(display_device::ActiveTopology { { "DeviceId1" } }))
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::topologyGuardFn(m_dd_api, { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}

TEST_F_S_MOCKED(TopologyGuardFn, Failure) {
  EXPECT_CALL(m_dd_api, setTopology(display_device::ActiveTopology { { "DeviceId1" } }))
    .Times(1)
    .WillOnce(Return(false))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::topologyGuardFn(m_dd_api, { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}

TEST_F_S_MOCKED(ModeGuardFn, Success) {
  EXPECT_CALL(m_dd_api, getCurrentDisplayModes(std::set<std::string> { "DeviceId1" }))
    .Times(1)
    .WillOnce(Return(display_device::DeviceDisplayModeMap { { "DeviceId1", {} } }))
    .RetiresOnSaturation();
  EXPECT_CALL(m_dd_api, setDisplayModes(display_device::DeviceDisplayModeMap { { "DeviceId1", {} } }))
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::modeGuardFn(m_dd_api, { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}

TEST_F_S_MOCKED(ModeGuardFn, Failure) {
  EXPECT_CALL(m_dd_api, getCurrentDisplayModes(std::set<std::string> { "DeviceId1" }))
    .Times(1)
    .WillOnce(Return(display_device::DeviceDisplayModeMap { { "DeviceId1", {} } }))
    .RetiresOnSaturation();
  EXPECT_CALL(m_dd_api, setDisplayModes(display_device::DeviceDisplayModeMap { { "DeviceId1", {} } }))
    .Times(1)
    .WillOnce(Return(false))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::modeGuardFn(m_dd_api, { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}

TEST_F_S_MOCKED(PrimaryGuardFn, Success) {
  EXPECT_CALL(m_dd_api, isPrimary("DeviceId1"))
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();
  EXPECT_CALL(m_dd_api, setAsPrimary("DeviceId1"))
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::primaryGuardFn(m_dd_api, display_device::ActiveTopology { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}

TEST_F_S_MOCKED(PrimaryGuardFn, Failure) {
  EXPECT_CALL(m_dd_api, isPrimary("DeviceId1"))
    .Times(1)
    .WillOnce(Return(false))
    .RetiresOnSaturation();
  EXPECT_CALL(m_dd_api, setAsPrimary(""))
    .Times(1)
    .WillOnce(Return(false))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::primaryGuardFn(m_dd_api, display_device::ActiveTopology { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}

TEST_F_S_MOCKED(HdrStateGuardFn, Success) {
  EXPECT_CALL(m_dd_api, getCurrentHdrStates(std::set<std::string> { "DeviceId1" }))
    .Times(1)
    .WillOnce(Return(display_device::HdrStateMap { { "DeviceId1", {} } }))
    .RetiresOnSaturation();
  EXPECT_CALL(m_dd_api, setHdrStates(display_device::HdrStateMap { { "DeviceId1", {} } }))
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::hdrStateGuardFn(m_dd_api, { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}

TEST_F_S_MOCKED(HdrStateGuardFn, Failure) {
  EXPECT_CALL(m_dd_api, getCurrentHdrStates(std::set<std::string> { "DeviceId1" }))
    .Times(1)
    .WillOnce(Return(display_device::HdrStateMap { { "DeviceId1", {} } }))
    .RetiresOnSaturation();
  EXPECT_CALL(m_dd_api, setHdrStates(display_device::HdrStateMap { { "DeviceId1", {} } }))
    .Times(1)
    .WillOnce(Return(false))
    .RetiresOnSaturation();

  const auto guard_fn { display_device::win_utils::hdrStateGuardFn(m_dd_api, { { "DeviceId1" } }) };
  EXPECT_NO_THROW(guard_fn());
}
