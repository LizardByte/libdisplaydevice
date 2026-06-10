// system includes
#include <string>
#include <string_view>

// local includes
#include "display_device/windows/settings_manager.h"
#include "display_device/windows/settings_utils.h"
#include "fixtures/fixtures.h"
#include "fixtures/mock_audio_context.h"
#include "fixtures/mock_settings_persistence.h"
#include "utils/helpers.h"
#include "utils/mock_win_display_device.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::HasSubstr;
  using ::testing::InSequence;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Additional convenience global const(s)
  const display_device::ActiveTopology DEFAULT_CURRENT_TOPOLOGY {{"DeviceId1", "DeviceId2"}, {"DeviceId3"}};
  const display_device::EnumeratedDeviceList DEFAULT_DEVICES {
    {.m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info {.m_primary = true}},
    {.m_device_id = "DeviceId2", .m_info = display_device::EnumeratedDevice::Info {.m_primary = true}},
    {.m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
    {.m_device_id = "DeviceId4"}
  };
  const display_device::EnumeratedDeviceList DEVICES_WITH_NO_PRIMARY {
    {.m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
    {.m_device_id = "DeviceId2", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
    {.m_device_id = "DeviceId3", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
    {.m_device_id = "DeviceId4"}
  };
  const display_device::DeviceDisplayModeMap DEFAULT_CURRENT_MODES {
    {"DeviceId1", {{1080, 720}, {120, 1}}},
    {"DeviceId2", {{1920, 1080}, {60, 1}}},
    {"DeviceId3", {{2560, 1440}, {30, 1}}}
  };
  const display_device::HdrStateMap DEFAULT_CURRENT_HDR_STATES {
    {"DeviceId1", display_device::HdrState::Disabled},
    {"DeviceId2", display_device::HdrState::Disabled},
    {"DeviceId3", std::nullopt}
  };
  const display_device::SingleDisplayConfigState DEFAULT_PERSISTENCE_INPUT_BASE {{DEFAULT_CURRENT_TOPOLOGY, {"DeviceId1", "DeviceId2"}}};

  // Test fixture(s) for this file
  class SettingsManagerApplyMocked: public BaseTest {
  public:
    display_device::SettingsManager &getImpl() {
      if (!m_impl) {
        m_impl = std::make_unique<display_device::SettingsManager>(m_dd_api, m_audio_context_api, std::make_unique<display_device::PersistentState>(m_settings_persistence_api), display_device::WinWorkarounds {
                                                                                                                                                                                   .m_hdr_blank_delay = std::chrono::milliseconds {123}  // Value is irrelevant for the tests
                                                                                                                                                                                 });
      }

      return *m_impl;
    }

    void expectedDefaultCallsUntilTopologyPrep(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::ActiveTopology &topology = DEFAULT_CURRENT_TOPOLOGY, const std::optional<display_device::SingleDisplayConfigState> &state = ut_consts::SDCS_EMPTY) const {
      EXPECT_CALL(*m_settings_persistence_api, load())
        .Times(1)
        .WillOnce(Return(serializeState(state)))
        .RetiresOnSaturation();
      EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
      EXPECT_CALL(*m_dd_api, getCurrentTopology())
        .Times(1)
        .WillOnce(Return(topology))
        .RetiresOnSaturation();
      EXPECT_CALL(*m_dd_api, isTopologyValid(topology))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    }

    void expectedIsCapturedCall(InSequence & /* To ensure that sequence is created outside this scope */, const bool is_captured) const {
      EXPECT_CALL(*m_audio_context_api, isCaptured())
        .Times(1)
        .WillOnce(Return(is_captured))
        .RetiresOnSaturation();
    }

    void expectedCaptureCall(InSequence & /* To ensure that sequence is created outside this scope */, const bool success = true) {
      EXPECT_CALL(*m_audio_context_api, capture())
        .Times(1)
        .WillOnce(Return(success))
        .RetiresOnSaturation();
    }

    void expectedReleaseCall(InSequence & /* To ensure that sequence is created outside this scope */) {
      EXPECT_CALL(*m_audio_context_api, release())
        .Times(1)
        .RetiresOnSaturation();
    }

    void expectedTopologyGuardTopologyCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::ActiveTopology &topology = DEFAULT_CURRENT_TOPOLOGY, const bool success = true) {
      EXPECT_CALL(*m_dd_api, setTopology(topology))
        .Times(1)
        .WillOnce(Return(success))
        .RetiresOnSaturation();
    }

    void expectedDeviceEnumCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::EnumeratedDeviceList &devices = DEFAULT_DEVICES) const {
      EXPECT_CALL(*m_dd_api, enumAvailableDevices())
        .Times(1)
        .WillOnce(Return(devices))
        .RetiresOnSaturation();
    }

    void expectedIsTopologyTheSameCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::ActiveTopology &lhs, const display_device::ActiveTopology &rhs) const {
      EXPECT_CALL(*m_dd_api, isTopologyTheSame(lhs, rhs))
        .Times(1)
        .WillOnce(Return(lhs == rhs))
        .RetiresOnSaturation();
    }

    void expectedSetTopologyCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::ActiveTopology &topology) {
      EXPECT_CALL(*m_dd_api, setTopology(topology))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    }

    void expectedPersistenceCall(InSequence & /* To ensure that sequence is created outside this scope */, const std::optional<display_device::SingleDisplayConfigState> &state, const bool success = true) {
      EXPECT_CALL(*m_settings_persistence_api, store(*serializeState(state)))
        .Times(1)
        .WillOnce(Return(success))
        .RetiresOnSaturation();
    }

    void expectedTopologyGuardNewlyCapturedContextCall(InSequence & /* To ensure that sequence is created outside this scope */, const bool is_captured) {
      EXPECT_CALL(*m_audio_context_api, isCaptured())
        .Times(1)
        .WillOnce(Return(is_captured))
        .RetiresOnSaturation();

      if (is_captured) {
        EXPECT_CALL(*m_audio_context_api, release())
          .Times(1)
          .RetiresOnSaturation();
      }
    }

    void expectedIsPrimaryCall(InSequence & /* To ensure that sequence is created outside this scope */, const std::string &device_id, const bool success = true) const {
      EXPECT_CALL(*m_dd_api, isPrimary(device_id))
        .Times(1)
        .WillOnce(Return(success))
        .RetiresOnSaturation();
    }

    void expectedSetAsPrimaryCall(InSequence & /* To ensure that sequence is created outside this scope */, const std::string &device_id, const bool success = true) {
      EXPECT_CALL(*m_dd_api, setAsPrimary(device_id))
        .Times(1)
        .WillOnce(Return(success))
        .RetiresOnSaturation();
    }

    void expectedPrimaryGuardCall(InSequence & /* To ensure that sequence is created outside this scope */, const std::string &device_id) {
      EXPECT_CALL(*m_dd_api, setAsPrimary(device_id))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    }

    void expectedSetDisplayModesCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::DeviceDisplayModeMap &modes, const bool success = true) {
      EXPECT_CALL(*m_dd_api, setDisplayModes(modes))
        .Times(1)
        .WillOnce(Return(success))
        .RetiresOnSaturation();
    }

    void expectedGetCurrentDisplayModesCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::StringSet &devices, const display_device::DeviceDisplayModeMap &modes) const {
      EXPECT_CALL(*m_dd_api, getCurrentDisplayModes(devices))
        .Times(1)
        .WillOnce(Return(modes))
        .RetiresOnSaturation();
    }

    void expectedSetDisplayModesGuardCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::DeviceDisplayModeMap &modes) {
      EXPECT_CALL(*m_dd_api, setDisplayModes(modes))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    }

    void expectedSetHdrStatesCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::HdrStateMap &states, const bool success = true) {
      EXPECT_CALL(*m_dd_api, setHdrStates(states))
        .Times(1)
        .WillOnce(Return(success))
        .RetiresOnSaturation();
    }

    void expectedGetCurrentHdrStatesCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::StringSet &devices, const display_device::HdrStateMap &states) const {
      EXPECT_CALL(*m_dd_api, getCurrentHdrStates(devices))
        .Times(1)
        .WillOnce(Return(states))
        .RetiresOnSaturation();
    }

    void expectedSetHdrStatesGuardCall(InSequence & /* To ensure that sequence is created outside this scope */, const display_device::HdrStateMap &states) {
      EXPECT_CALL(*m_dd_api, setHdrStates(states))
        .Times(1)
        .WillOnce(Return(true))
        .RetiresOnSaturation();
    }

    void expectedHdrWorkaroundCalls(InSequence & /* To ensure that sequence is created outside this scope */) const {
      // Using the "failure" path, to keep it simple
      EXPECT_CALL(*m_dd_api, getCurrentTopology())
        .Times(1)
        .WillOnce(Return(display_device::ActiveTopology {}))
        .RetiresOnSaturation();
      EXPECT_CALL(*m_dd_api, isTopologyValid(display_device::ActiveTopology {}))
        .Times(1)
        .WillOnce(Return(false))
        .RetiresOnSaturation();
    }

    void expectedStableTopologyPrepCalls(InSequence &sequence /* To ensure that sequence is created outside this scope */, const std::optional<display_device::SingleDisplayConfigState> &state = ut_consts::SDCS_EMPTY) const {
      expectedDefaultCallsUntilTopologyPrep(sequence, DEFAULT_CURRENT_TOPOLOGY, state);
      expectedIsCapturedCall(sequence, false);
      expectedDeviceEnumCall(sequence);
      expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
    }

    void expectedChangedTopologyPrepCalls(InSequence &sequence /* To ensure that sequence is created outside this scope */, const display_device::ActiveTopology &modified_topology) {
      expectedDefaultCallsUntilTopologyPrep(sequence);
      expectedIsCapturedCall(sequence, false);
      expectedDeviceEnumCall(sequence);
      expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, modified_topology);
      expectedIsCapturedCall(sequence, false);
      expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
      expectedSetTopologyCall(sequence, modified_topology);
      expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, modified_topology);
    }

    void expectedDisplayModeChangeCalls(InSequence &sequence /* To ensure that sequence is created outside this scope */, const display_device::DeviceDisplayModeMap &new_modes, const display_device::DeviceDisplayModeMap &verified_modes) {
      expectedGetCurrentDisplayModesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_MODES);
      expectedSetDisplayModesCall(sequence, new_modes);
      expectedGetCurrentDisplayModesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), verified_modes);
    }

    void expectedHdrStateChangeCalls(InSequence &sequence /* To ensure that sequence is created outside this scope */, const display_device::HdrStateMap &new_states) {
      expectedGetCurrentHdrStatesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_HDR_STATES);
      expectedSetHdrStatesCall(sequence, new_states);
    }

    void expectedAudioDelayedReleasePrepCalls(InSequence &sequence /* To ensure that sequence is created outside this scope */, const display_device::SingleDisplayConfigState &persistence_input) {
      expectedDefaultCallsUntilTopologyPrep(sequence, DEFAULT_CURRENT_TOPOLOGY, ut_consts::SDCS_NO_MODIFICATIONS);
      expectedIsCapturedCall(sequence, false);
      expectedDeviceEnumCall(sequence);
      expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, {{"DeviceId1"}});
      expectedIsTopologyTheSameCall(sequence, ut_consts::SDCS_FULL->m_modified.m_topology, {{"DeviceId1"}});

      expectedIsCapturedCall(sequence, true);
      expectedSetTopologyCall(sequence, persistence_input.m_initial.m_topology);
      expectedIsTopologyTheSameCall(sequence, ut_consts::SDCS_FULL->m_initial.m_topology, {{"DeviceId1"}});
    }

    void expectedPrimaryRestorePrepCalls(InSequence &sequence /* To ensure that sequence is created outside this scope */, const std::optional<display_device::SingleDisplayConfigState> &initial_state) const {
      expectedDefaultCallsUntilTopologyPrep(sequence, initial_state->m_modified.m_topology, initial_state);
      expectedIsCapturedCall(sequence, false);
      expectedDeviceEnumCall(sequence);
      expectedIsTopologyTheSameCall(sequence, initial_state->m_modified.m_topology, initial_state->m_modified.m_topology);

      expectedIsPrimaryCall(sequence, "DeviceId1", false);
      expectedIsPrimaryCall(sequence, "DeviceId3");
    }

    std::shared_ptr<StrictMock<display_device::MockWinDisplayDevice>> m_dd_api {std::make_shared<StrictMock<display_device::MockWinDisplayDevice>>()};
    std::shared_ptr<StrictMock<display_device::MockSettingsPersistence>> m_settings_persistence_api {std::make_shared<StrictMock<display_device::MockSettingsPersistence>>()};
    std::shared_ptr<StrictMock<display_device::MockAudioContext>> m_audio_context_api {std::make_shared<StrictMock<display_device::MockAudioContext>>()};
    std::unique_ptr<display_device::SettingsManager> m_impl;
  };

  display_device::SingleDisplayConfigState makePrimaryDevicePersistenceInput(const std::string_view original_primary_device) {
    auto state {DEFAULT_PERSISTENCE_INPUT_BASE};
    state.m_modified.m_topology = {{"DeviceId1", "DeviceId2"}, {"DeviceId3"}, {"DeviceId4"}};
    state.m_modified.m_original_primary_device = std::string {original_primary_device};
    return state;
  }

  display_device::SingleDisplayConfigState makeDisplayModePersistenceInput() {
    auto state {DEFAULT_PERSISTENCE_INPUT_BASE};
    state.m_modified.m_topology = DEFAULT_CURRENT_TOPOLOGY;
    state.m_modified.m_original_modes = DEFAULT_CURRENT_MODES;
    return state;
  }

  display_device::SingleDisplayConfigState makeTopologyOnlyPersistenceInput() {
    auto state {DEFAULT_PERSISTENCE_INPUT_BASE};
    state.m_modified.m_topology = DEFAULT_CURRENT_TOPOLOGY;
    return state;
  }

  display_device::DeviceDisplayModeMap makeModesWithDevice1Resolution() {
    auto modes {DEFAULT_CURRENT_MODES};
    modes["DeviceId1"].m_resolution = {1920, 1080};
    return modes;
  }

  display_device::DeviceDisplayModeMap makeModesWithDevice1RefreshRate() {
    auto modes {DEFAULT_CURRENT_MODES};
    modes["DeviceId1"].m_refresh_rate = {308500, 10000};
    return modes;
  }

  display_device::DeviceDisplayModeMap makeModesWithDevice1ResolutionAndRefreshRate() {
    auto modes {makeModesWithDevice1Resolution()};
    modes["DeviceId1"].m_refresh_rate = {308500, 10000};
    return modes;
  }

  display_device::DeviceDisplayModeMap makeModesWithPrimaryResolutionAndRefreshRate() {
    auto modes {makeModesWithDevice1ResolutionAndRefreshRate()};
    modes["DeviceId2"].m_resolution = {1920, 1080};
    modes["DeviceId2"].m_refresh_rate = {308500, 10000};
    return modes;
  }

  display_device::SingleDisplayConfigState makeHdrStatePersistenceInput() {
    auto state {DEFAULT_PERSISTENCE_INPUT_BASE};
    state.m_modified.m_topology = DEFAULT_CURRENT_TOPOLOGY;
    state.m_modified.m_original_hdr_states = DEFAULT_CURRENT_HDR_STATES;
    return state;
  }

  display_device::HdrStateMap makeHdrStatesWithDevice1Enabled() {
    auto states {DEFAULT_CURRENT_HDR_STATES};
    states["DeviceId1"] = display_device::HdrState::Enabled;
    return states;
  }

  display_device::HdrStateMap makeHdrStatesWithPrimaryDevicesEnabled() {
    auto states {makeHdrStatesWithDevice1Enabled()};
    states["DeviceId2"] = display_device::HdrState::Enabled;
    return states;
  }

  display_device::SingleDisplayConfigState makeDisplayModeRestoreInitialState() {
    auto state {makeDisplayModePersistenceInput()};
    state.m_modified.m_original_modes["DeviceId1"].m_resolution = {1920, 1080};
    return state;
  }

  display_device::SingleDisplayConfigState makeHdrStateRestoreInitialState() {
    auto state {makeHdrStatePersistenceInput()};
    state.m_modified.m_original_hdr_states["DeviceId1"] = display_device::HdrState::Enabled;
    return state;
  }

  display_device::SingleDisplayConfigState makeAudioDelayedReleasePersistenceInput() {
    auto state {*ut_consts::SDCS_NO_MODIFICATIONS};
    state.m_modified = {{{"DeviceId1"}}};
    return state;
  }

  std::optional<display_device::SingleDisplayConfigState> makePrimaryRestoreInitialState() {
    auto state {ut_consts::SDCS_NO_MODIFICATIONS};
    state->m_modified.m_original_primary_device = "DeviceId1";
    return state;
  }

  // Specialized TEST macro(s) for this test
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, SettingsManagerApplyMocked, __VA_ARGS__)
}  // namespace

TEST_F_S_MOCKED(NoApiAccess) {
  InSequence sequence;
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(ut_consts::SDCS_EMPTY)));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().applySettings({}), display_device::SettingsManager::ApplyResult::ApiTemporarilyUnavailable);
}

TEST_F_S_MOCKED(CurrentTopologyIsInvalid) {
  InSequence sequence;
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(ut_consts::SDCS_EMPTY)));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_dd_api, getCurrentTopology())
    .Times(1)
    .WillOnce(Return(DEFAULT_CURRENT_TOPOLOGY));
  EXPECT_CALL(*m_dd_api, isTopologyValid(DEFAULT_CURRENT_TOPOLOGY))
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().applySettings({}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, FailedToEnumerateDevices) {
  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  EXPECT_CALL(*m_dd_api, enumAvailableDevices())
    .Times(1)
    .WillOnce(Return(display_device::EnumeratedDeviceList {}))
    .RetiresOnSaturation();

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, DeviceNotAvailable) {
  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceIdX"}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, FailedToComputeInitialState) {
  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence, DEVICES_WITH_NO_PRIMARY);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, FailedToStripInitialState) {
  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence, {{.m_device_id = "DeviceId4", .m_info = display_device::EnumeratedDevice::Info {.m_primary = true}}});

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4"}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, DeviceNotFoundInNewTopology) {
  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4"}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, NoChangeIsNeeded) {
  auto persistence_input {DEFAULT_PERSISTENCE_INPUT_BASE};
  persistence_input.m_modified = {DEFAULT_CURRENT_TOPOLOGY};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedPersistenceCall(sequence, persistence_input);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareTopology, RevertingSettingsFailed) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence, DEFAULT_CURRENT_TOPOLOGY, ut_consts::SDCS_FULL);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, {{"DeviceId1"}});
  expectedIsTopologyTheSameCall(sequence, ut_consts::SDCS_FULL->m_modified.m_topology, {{"DeviceId1"}});
  EXPECT_CALL(*m_dd_api, isTopologyValid(ut_consts::SDCS_FULL->m_modified.m_topology))
    .Times(1)
    .WillOnce(Return(false))
    .RetiresOnSaturation();

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, AudioContextCaptureFailed) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, {{"DeviceId1"}});
  expectedIsCapturedCall(sequence, false);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedCaptureCall(sequence, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, TopologyChangeFailed) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, {{"DeviceId1"}});
  expectedIsCapturedCall(sequence, false);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedCaptureCall(sequence, true);
  EXPECT_CALL(*m_dd_api, setTopology(display_device::ActiveTopology {{"DeviceId1"}}))
    .Times(1)
    .WillOnce(Return(false))
    .RetiresOnSaturation();

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, true);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S_MOCKED(PrepareTopology, AudioContextCaptured) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  auto persistence_input {DEFAULT_PERSISTENCE_INPUT_BASE};
  persistence_input.m_modified = {{{"DeviceId1"}}};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, {{"DeviceId1"}});
  expectedIsCapturedCall(sequence, false);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedCaptureCall(sequence, true);
  expectedSetTopologyCall(sequence, {{"DeviceId1"}});
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, {{"DeviceId1"}});
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareTopology, AudioContextCaptureSkipped, NotInitialTopologySwitch) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  auto persistence_input {*ut_consts::SDCS_NO_MODIFICATIONS};
  persistence_input.m_modified = {{{"DeviceId1"}}};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence, DEFAULT_CURRENT_TOPOLOGY, ut_consts::SDCS_NO_MODIFICATIONS);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, {{"DeviceId1"}});
  expectedIsTopologyTheSameCall(sequence, ut_consts::SDCS_FULL->m_modified.m_topology, {{"DeviceId1"}});

  expectedIsCapturedCall(sequence, false);
  expectedIsTopologyTheSameCall(sequence, ut_consts::SDCS_FULL->m_initial.m_topology, DEFAULT_CURRENT_TOPOLOGY);
  expectedSetTopologyCall(sequence, {{"DeviceId1"}});
  expectedIsTopologyTheSameCall(sequence, ut_consts::SDCS_FULL->m_initial.m_topology, {{"DeviceId1"}});
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareTopology, AudioContextCaptureSkipped, NoDevicesAreGone) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  auto persistence_input {DEFAULT_PERSISTENCE_INPUT_BASE};
  persistence_input.m_modified = {{{"DeviceId1", "DeviceId2"}, {"DeviceId3"}, {"DeviceId4"}}};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, persistence_input.m_modified.m_topology);
  expectedIsCapturedCall(sequence, false);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedSetTopologyCall(sequence, persistence_input.m_modified.m_topology);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, persistence_input.m_modified.m_topology);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4", .m_device_prep = DevicePrep::EnsureActive}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, FailedToGetPrimaryDevice) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const display_device::ActiveTopology topology {{"DeviceId1", "DeviceId2"}, {"DeviceId3"}, {"DeviceId4"}};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, topology);
  expectedIsCapturedCall(sequence, false);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedSetTopologyCall(sequence, topology);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, topology);

  expectedIsPrimaryCall(sequence, "DeviceId1", false);
  expectedIsPrimaryCall(sequence, "DeviceId2", false);
  expectedIsPrimaryCall(sequence, "DeviceId3", false);
  expectedIsPrimaryCall(sequence, "DeviceId4", false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4", .m_device_prep = DevicePrep::EnsurePrimary}), display_device::SettingsManager::ApplyResult::PrimaryDevicePrepFailed);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, FailedToSetPrimaryDevice) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const display_device::ActiveTopology topology {{"DeviceId1", "DeviceId2"}, {"DeviceId3"}, {"DeviceId4"}};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, topology);
  expectedIsCapturedCall(sequence, false);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedSetTopologyCall(sequence, topology);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, topology);

  expectedIsPrimaryCall(sequence, "DeviceId1");
  expectedSetAsPrimaryCall(sequence, "DeviceId4", false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4", .m_device_prep = DevicePrep::EnsurePrimary}), display_device::SettingsManager::ApplyResult::PrimaryDevicePrepFailed);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, PrimaryDeviceSet) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto persistence_input {makePrimaryDevicePersistenceInput("DeviceId1")};

  InSequence sequence;
  expectedChangedTopologyPrepCalls(sequence, persistence_input.m_modified.m_topology);

  expectedIsPrimaryCall(sequence, "DeviceId1");
  expectedSetAsPrimaryCall(sequence, "DeviceId4");
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4", .m_device_prep = DevicePrep::EnsurePrimary}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, PrimaryDeviceSet, CachedDeviceReused) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  auto initial_state {DEFAULT_PERSISTENCE_INPUT_BASE};
  initial_state.m_modified.m_topology = {{"DeviceId1", "DeviceId2"}, {"DeviceId3"}, {"DeviceId4"}};
  initial_state.m_modified.m_original_primary_device = "DeviceId1";

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence, initial_state.m_modified.m_topology, initial_state);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, initial_state.m_modified.m_topology, initial_state.m_modified.m_topology);

  expectedIsPrimaryCall(sequence, "DeviceId1", false);
  expectedIsPrimaryCall(sequence, "DeviceId2", false);
  expectedIsPrimaryCall(sequence, "DeviceId3");
  expectedSetAsPrimaryCall(sequence, "DeviceId4");
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4", .m_device_prep = DevicePrep::EnsurePrimary}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, PrimaryDeviceSet, GuardInvoked) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto persistence_input {makePrimaryDevicePersistenceInput("DeviceId1")};

  InSequence sequence;
  expectedChangedTopologyPrepCalls(sequence, persistence_input.m_modified.m_topology);

  expectedIsPrimaryCall(sequence, "DeviceId1");
  expectedSetAsPrimaryCall(sequence, "DeviceId4");
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedPrimaryGuardCall(sequence, "DeviceId1");
  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4", .m_device_prep = DevicePrep::EnsurePrimary}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, PrimaryDeviceSetSkipped) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto persistence_input {makePrimaryDevicePersistenceInput("DeviceId4")};

  InSequence sequence;
  expectedChangedTopologyPrepCalls(sequence, persistence_input.m_modified.m_topology);

  expectedIsPrimaryCall(sequence, "DeviceId1", false);
  expectedIsPrimaryCall(sequence, "DeviceId2", false);
  expectedIsPrimaryCall(sequence, "DeviceId3", false);
  expectedIsPrimaryCall(sequence, "DeviceId4");
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId4", .m_device_prep = DevicePrep::EnsurePrimary}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, FailedToRestorePrimaryDevice) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto initial_state {makePrimaryRestoreInitialState()};

  InSequence sequence;
  expectedPrimaryRestorePrepCalls(sequence, initial_state);
  expectedSetAsPrimaryCall(sequence, "DeviceId1", false);

  expectedTopologyGuardTopologyCall(sequence, initial_state->m_modified.m_topology);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId3", .m_device_prep = DevicePrep::EnsureActive}), display_device::SettingsManager::ApplyResult::PrimaryDevicePrepFailed);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, PrimaryDeviceRestored) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto initial_state {makePrimaryRestoreInitialState()};

  auto persistence_input {initial_state};
  persistence_input->m_modified.m_original_primary_device = "";

  InSequence sequence;
  expectedPrimaryRestorePrepCalls(sequence, initial_state);
  expectedSetAsPrimaryCall(sequence, "DeviceId1", true);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId3", .m_device_prep = DevicePrep::EnsureActive}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, PrimaryDeviceRestored, PersistenceFailed) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto initial_state {makePrimaryRestoreInitialState()};

  auto persistence_input {initial_state};
  persistence_input->m_modified.m_original_primary_device = "";

  InSequence sequence;
  expectedPrimaryRestorePrepCalls(sequence, initial_state);
  expectedSetAsPrimaryCall(sequence, "DeviceId1", true);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedPrimaryGuardCall(sequence, "DeviceId3");
  expectedTopologyGuardTopologyCall(sequence, initial_state->m_modified.m_topology);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId3", .m_device_prep = DevicePrep::EnsureActive}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PreparePrimaryDevice, PrimaryDeviceRestoreSkipped, PersistenceFailed) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  auto intial_state {ut_consts::SDCS_NO_MODIFICATIONS};
  intial_state->m_modified.m_original_primary_device = "DeviceId1";

  auto persistence_input {intial_state};
  persistence_input->m_modified.m_original_primary_device = "";

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence, intial_state->m_modified.m_topology, intial_state);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, intial_state->m_modified.m_topology, intial_state->m_modified.m_topology);

  expectedIsPrimaryCall(sequence, "DeviceId1");
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence, intial_state->m_modified.m_topology);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId3", .m_device_prep = DevicePrep::EnsureActive}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareDisplayModes, FailedToGetDisplayModes) {
  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);

  expectedGetCurrentDisplayModesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), {});

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_resolution = {{1920, 1080}}}), display_device::SettingsManager::ApplyResult::DisplayModePrepFailed);
}

TEST_F_S_MOCKED(PrepareDisplayModes, FailedToSetDisplayModes) {
  const auto new_modes {makeModesWithDevice1Resolution()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);

  expectedGetCurrentDisplayModesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_MODES);
  expectedSetDisplayModesCall(sequence, new_modes, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_resolution = {{1920, 1080}}}), display_device::SettingsManager::ApplyResult::DisplayModePrepFailed);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSet, ResolutionOnly) {
  const auto new_modes {makeModesWithDevice1Resolution()};
  const auto persistence_input {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedDisplayModeChangeCalls(sequence, new_modes, new_modes);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_resolution = {{1920, 1080}}}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSet, RefreshRateOnly) {
  const auto new_modes {makeModesWithDevice1RefreshRate()};
  const auto persistence_input {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedDisplayModeChangeCalls(sequence, new_modes, new_modes);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_refresh_rate = {{30.85}}}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSet, ResolutionAndRefreshRate) {
  const auto new_modes {makeModesWithDevice1ResolutionAndRefreshRate()};
  const auto persistence_input {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedDisplayModeChangeCalls(sequence, new_modes, new_modes);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_resolution = {{1920, 1080}}, .m_refresh_rate = {{30.85}}}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSet, ResolutionAndRefreshRate, PrimaryDeviceSpecified) {
  const auto new_modes {makeModesWithPrimaryResolutionAndRefreshRate()};
  const auto persistence_input {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedDisplayModeChangeCalls(sequence, new_modes, new_modes);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_resolution = {{1920, 1080}}, .m_refresh_rate = {{30.85}}}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSet, CachedModesReused) {
  const auto new_modes {makeModesWithDevice1Resolution()};
  const auto initial_state {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);
  expectedDisplayModeChangeCalls(sequence, new_modes, new_modes);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_resolution = {{1920, 1080}}}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSet, GuardInvoked) {
  const auto new_modes {makeModesWithDevice1Resolution()};
  const auto persistence_input {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedDisplayModeChangeCalls(sequence, new_modes, new_modes);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedSetDisplayModesGuardCall(sequence, DEFAULT_CURRENT_MODES);
  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_resolution = {{1920, 1080}}}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSet, GuardNotInvoked) {
  const auto new_modes {makeModesWithDevice1Resolution()};
  const auto persistence_input {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedDisplayModeChangeCalls(sequence, new_modes, DEFAULT_CURRENT_MODES);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_resolution = {{1920, 1080}}}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesSetSkipped) {
  const auto persistence_input {makeDisplayModePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);

  expectedGetCurrentDisplayModesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_MODES);
  expectedPersistenceCall(sequence, persistence_input);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId3", .m_resolution = {{2560, 1440}}}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareDisplayModes, FailedToRestoreDisplayModes) {
  const auto initial_state {makeDisplayModeRestoreInitialState()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);

  expectedGetCurrentDisplayModesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_MODES);
  expectedSetDisplayModesCall(sequence, initial_state.m_modified.m_original_modes, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::DisplayModePrepFailed);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesRestored) {
  const auto initial_state {makeDisplayModeRestoreInitialState()};
  const auto persistence_input {makeTopologyOnlyPersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);
  expectedDisplayModeChangeCalls(sequence, initial_state.m_modified.m_original_modes, initial_state.m_modified.m_original_modes);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesRestored, PersistenceFailed) {
  const auto initial_state {makeDisplayModeRestoreInitialState()};
  const auto persistence_input {makeTopologyOnlyPersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);
  expectedDisplayModeChangeCalls(sequence, initial_state.m_modified.m_original_modes, initial_state.m_modified.m_original_modes);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedSetDisplayModesGuardCall(sequence, DEFAULT_CURRENT_MODES);
  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareDisplayModes, DisplayModesRestoreSkipped, PersistenceFailed) {
  const auto initial_state {makeDisplayModePersistenceInput()};
  const auto persistence_input {makeTopologyOnlyPersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);

  expectedGetCurrentDisplayModesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_MODES);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareHdrStates, FailedToGetHdrStates) {
  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);

  expectedGetCurrentHdrStatesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), {});

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_hdr_state = display_device::HdrState::Enabled}), display_device::SettingsManager::ApplyResult::HdrStatePrepFailed);
}

TEST_F_S_MOCKED(PrepareHdrStates, FailedToSetHdrStates) {
  const auto new_states {makeHdrStatesWithDevice1Enabled()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);

  expectedGetCurrentHdrStatesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_HDR_STATES);
  expectedSetHdrStatesCall(sequence, new_states, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_hdr_state = display_device::HdrState::Enabled}), display_device::SettingsManager::ApplyResult::HdrStatePrepFailed);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesSet) {
  const auto new_states {makeHdrStatesWithDevice1Enabled()};
  const auto persistence_input {makeHdrStatePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedHdrStateChangeCalls(sequence, new_states);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_hdr_state = display_device::HdrState::Enabled}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesSet, PrimaryDeviceSpecified) {
  const auto new_states {makeHdrStatesWithPrimaryDevicesEnabled()};
  const auto persistence_input {makeHdrStatePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedHdrStateChangeCalls(sequence, new_states);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_hdr_state = display_device::HdrState::Enabled}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesSet, CachedModesReused) {
  const auto new_states {makeHdrStatesWithDevice1Enabled()};
  const auto initial_state {makeHdrStatePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);
  expectedHdrStateChangeCalls(sequence, new_states);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_hdr_state = display_device::HdrState::Enabled}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesSet, GuardInvoked) {
  const auto new_states {makeHdrStatesWithDevice1Enabled()};
  const auto persistence_input {makeHdrStatePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedHdrStateChangeCalls(sequence, new_states);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedSetHdrStatesGuardCall(sequence, DEFAULT_CURRENT_HDR_STATES);
  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_hdr_state = display_device::HdrState::Enabled}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesSetSkipped) {
  const auto persistence_input {makeHdrStatePersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);

  expectedGetCurrentHdrStatesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_HDR_STATES);
  expectedPersistenceCall(sequence, persistence_input);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId3", .m_hdr_state = display_device::HdrState::Enabled}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareHdrStates, FailedToRestoreHdrStates) {
  const auto initial_state {makeHdrStateRestoreInitialState()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);

  expectedGetCurrentHdrStatesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_HDR_STATES);
  expectedSetHdrStatesCall(sequence, initial_state.m_modified.m_original_hdr_states, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::HdrStatePrepFailed);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesRestored) {
  const auto initial_state {makeHdrStateRestoreInitialState()};
  const auto persistence_input {makeTopologyOnlyPersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);
  expectedHdrStateChangeCalls(sequence, initial_state.m_modified.m_original_hdr_states);
  expectedPersistenceCall(sequence, persistence_input);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesRestored, PersistenceFailed) {
  const auto initial_state {makeHdrStateRestoreInitialState()};
  const auto persistence_input {makeTopologyOnlyPersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);
  expectedHdrStateChangeCalls(sequence, initial_state.m_modified.m_original_hdr_states);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedSetHdrStatesGuardCall(sequence, DEFAULT_CURRENT_HDR_STATES);
  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareHdrStates, HdrStatesRestoreSkipped, PersistenceFailed) {
  const auto initial_state {makeHdrStatePersistenceInput()};
  const auto persistence_input {makeTopologyOnlyPersistenceInput()};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence, initial_state);

  expectedGetCurrentHdrStatesCall(sequence, display_device::win_utils::flattenTopology(DEFAULT_CURRENT_TOPOLOGY), DEFAULT_CURRENT_HDR_STATES);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(AudioContextDelayedRelease) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto persistence_input {makeAudioDelayedReleasePersistenceInput()};

  InSequence sequence;
  expectedAudioDelayedReleasePrepCalls(sequence, persistence_input);
  expectedPersistenceCall(sequence, persistence_input);
  expectedReleaseCall(sequence);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::Ok);
}

TEST_F_S_MOCKED(FailedToSaveNewState) {
  auto persistence_input {DEFAULT_PERSISTENCE_INPUT_BASE};
  persistence_input.m_modified = {DEFAULT_CURRENT_TOPOLOGY};

  InSequence sequence;
  expectedStableTopologyPrepCalls(sequence);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(AudioContextDelayedRelease, ViaGuard) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto persistence_input {makeAudioDelayedReleasePersistenceInput()};

  InSequence sequence;
  expectedAudioDelayedReleasePrepCalls(sequence, persistence_input);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence, DEFAULT_CURRENT_TOPOLOGY, false);
  expectedReleaseCall(sequence);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(AudioContextDelayedRelease, SkippedDueToFailure) {
  using DevicePrep = display_device::SingleDisplayConfiguration::DevicePreparation;
  const auto persistence_input {makeAudioDelayedReleasePersistenceInput()};

  InSequence sequence;
  expectedAudioDelayedReleasePrepCalls(sequence, persistence_input);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence, DEFAULT_CURRENT_TOPOLOGY, true);
  expectedTopologyGuardNewlyCapturedContextCall(sequence, false);
  expectedHdrWorkaroundCalls(sequence);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1", .m_device_prep = DevicePrep::EnsureOnlyDisplay}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(TopologyGuardFailedButNoContextIsReleased) {
  auto persistence_input {DEFAULT_PERSISTENCE_INPUT_BASE};
  persistence_input.m_modified = {DEFAULT_CURRENT_TOPOLOGY};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, true);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedPersistenceCall(sequence, persistence_input, false);

  expectedTopologyGuardTopologyCall(sequence, DEFAULT_CURRENT_TOPOLOGY, false);

  EXPECT_EQ(getImpl().applySettings({.m_device_id = "DeviceId1"}), display_device::SettingsManager::ApplyResult::PersistenceSaveFailed);
}

TEST_F_S_MOCKED(PrepareTopology, PrimaryDeviceIsUsed) {
  auto persistence_input {DEFAULT_PERSISTENCE_INPUT_BASE};
  persistence_input.m_modified = {DEFAULT_CURRENT_TOPOLOGY};

  InSequence sequence;
  expectedDefaultCallsUntilTopologyPrep(sequence);
  expectedIsCapturedCall(sequence, false);
  expectedDeviceEnumCall(sequence);
  expectedIsTopologyTheSameCall(sequence, DEFAULT_CURRENT_TOPOLOGY, DEFAULT_CURRENT_TOPOLOGY);
  expectedPersistenceCall(sequence, persistence_input);

  EXPECT_EQ(getImpl().applySettings({}), display_device::SettingsManager::ApplyResult::Ok);
}
