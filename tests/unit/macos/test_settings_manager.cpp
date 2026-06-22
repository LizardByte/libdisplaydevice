// system includes
#include <stdexcept>

// local includes
#include "display_device/macos/json.h"
#include "display_device/macos/settings_manager.h"
#include "display_device/noop_audio_context.h"
#include "fixtures/fixtures.h"
#include "fixtures/mock_audio_context.h"
#include "fixtures/mock_settings_persistence.h"
#include "utils/mock_mac_display_device.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::HasSubstr;
  using ::testing::InSequence;
  using ::testing::Return;
  using ::testing::Sequence;
  using ::testing::StrictMock;

  const display_device::MacActiveTopology DEFAULT_TOPOLOGY {
    {"DeviceId1"},
    {"DeviceId2"}
  };
  const display_device::EnumeratedDeviceList DEFAULT_DEVICES {
    {.m_device_id = "DeviceId1", .m_info = display_device::EnumeratedDevice::Info {.m_primary = true}},
    {.m_device_id = "DeviceId2", .m_info = display_device::EnumeratedDevice::Info {.m_primary = false}},
    {.m_device_id = "DeviceId3"},
  };
  const display_device::MacDeviceDisplayModeMap DEFAULT_MODES {
    {"DeviceId1", {{1920, 1080}, {60, 1}}},
    {"DeviceId2", {{2560, 1440}, {120, 1}}},
  };
  const display_device::MacDeviceDisplayModeMap CHANGED_MODES {
    {"DeviceId1", {{1280, 720}, {60, 1}}},
    {"DeviceId2", {{2560, 1440}, {120, 1}}},
  };
  const display_device::MacDeviceDisplayModeMap CURRENT_REVERT_MODES {
    {"DeviceId1", {{1280, 720}, {60, 1}}},
    {"DeviceId2", {{2560, 1440}, {120, 1}}},
  };

  std::optional<std::vector<std::uint8_t>> serializeState(const std::optional<display_device::MacSingleDisplayConfigState> &state) {
    if (state) {
      bool is_ok {false};
      const auto data_string {display_device::toJson(*state, 2, &is_ok)};
      if (is_ok) {
        return std::vector<std::uint8_t> {std::begin(data_string), std::end(data_string)};
      }
    }

    return std::nullopt;
  }

  std::optional<std::vector<std::uint8_t>> serializeNoState() {
    return std::vector<std::uint8_t> {};
  }

  display_device::MacSingleDisplayConfigState makeState() {
    return {
      {{{"DeviceId1"}},
       {"DeviceId1"}},
      {display_device::MacSingleDisplayConfigState::Modified {
        {{"DeviceId2"}},
        {{"DeviceId2", {{1920, 1080}, {60, 1}}}},
        {{"DeviceId2", {std::nullopt}}},
        {"DeviceId1"},
      }}
    };
  }

  display_device::MacSingleDisplayConfigState makeModeState() {
    return {
      {DEFAULT_TOPOLOGY,
       {"DeviceId1"}},
      {display_device::MacSingleDisplayConfigState::Modified {
        DEFAULT_TOPOLOGY,
        DEFAULT_MODES,
        {},
        {},
      }}
    };
  }

  display_device::MacSingleDisplayConfigState makeAppliedModeState() {
    return makeModeState();
  }

  // Test fixture(s) for this file
  class MacSettingsManagerMocked: public BaseTest {
  public:
    display_device::MacSettingsManager &getImpl() {
      if (!m_impl) {
        m_impl = std::make_unique<display_device::MacSettingsManager>(
          m_dd_api,
          m_audio_context_api,
          std::make_unique<display_device::MacPersistentState>(m_settings_persistence_api),
          display_device::MacWorkarounds {}
        );
      }

      return *m_impl;
    }

    void expectNoStateLoad() const {
      EXPECT_CALL(*m_settings_persistence_api, load())
        .Times(1)
        .WillOnce(Return(serializeNoState()));
    }

    void expectStoredStateLoad(const display_device::MacSingleDisplayConfigState &state) const {
      EXPECT_CALL(*m_settings_persistence_api, load())
        .Times(1)
        .WillOnce(Return(serializeState(state)));
    }

    void expectApiAvailable(const Sequence &sequence) const {
      EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(true));
    }

    void expectApplyPreparation(const Sequence &sequence) const {
      expectApiAvailable(sequence);
      EXPECT_CALL(*m_dd_api, getCurrentTopology())
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(DEFAULT_TOPOLOGY));
      EXPECT_CALL(*m_dd_api, isTopologyValid(DEFAULT_TOPOLOGY))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(true));
      EXPECT_CALL(*m_dd_api, enumAvailableDevices())
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(DEFAULT_DEVICES));
    }

    void expectCurrentModes(const Sequence &sequence, const display_device::MacDeviceDisplayModeMap &modes) const {
      EXPECT_CALL(*m_dd_api, getCurrentDisplayModes(display_device::StringSet {"DeviceId1", "DeviceId2"}))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(modes));
    }

    void expectSetModes(const Sequence &sequence, const display_device::MacDeviceDisplayModeMap &modes, const bool result) const {
      EXPECT_CALL(*m_dd_api, setDisplayModes(modes))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(result));
    }

    void expectStoreState(const Sequence &sequence, const display_device::MacSingleDisplayConfigState &state, const bool result) const {
      EXPECT_CALL(*m_settings_persistence_api, store(*serializeState(state)))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(result));
    }

    void expectModeRevertPreparation(const Sequence &sequence, const display_device::MacDeviceDisplayModeMap &current_modes) const {
      expectApiAvailable(sequence);
      EXPECT_CALL(*m_dd_api, getCurrentTopology())
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(DEFAULT_TOPOLOGY));
      EXPECT_CALL(*m_dd_api, isTopologyValid(DEFAULT_TOPOLOGY))
        .Times(2)
        .InSequence(sequence)
        .WillRepeatedly(Return(true));
      EXPECT_CALL(*m_dd_api, isTopologyTheSame(DEFAULT_TOPOLOGY, DEFAULT_TOPOLOGY))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(true));
      expectCurrentModes(sequence, current_modes);
      expectSetModes(sequence, DEFAULT_MODES, true);
    }

    std::shared_ptr<StrictMock<display_device::MockMacDisplayDevice>> m_dd_api {std::make_shared<StrictMock<display_device::MockMacDisplayDevice>>()};
    std::shared_ptr<StrictMock<display_device::MockSettingsPersistence>> m_settings_persistence_api {std::make_shared<StrictMock<display_device::MockSettingsPersistence>>()};
    std::shared_ptr<StrictMock<display_device::MockAudioContext>> m_audio_context_api {std::make_shared<StrictMock<display_device::MockAudioContext>>()};
    std::unique_ptr<display_device::MacSettingsManager> m_impl;
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, MacSettingsManagerMocked, __VA_ARGS__)
}  // namespace

TEST_F_S(NullptrDisplayDeviceApiProvided) {
  EXPECT_THAT([]() {
    const display_device::MacSettingsManager settings_manager(nullptr, nullptr, nullptr, {});
  },
              ThrowsMessage<std::invalid_argument>(HasSubstr("Nullptr provided for MacDisplayDeviceInterface in MacSettingsManager!")));
}

TEST_F_S(NoopAudioContext) {
  const display_device::MacSettingsManager settings_manager {m_dd_api, nullptr, std::make_unique<display_device::MacPersistentState>(nullptr), {}};
  EXPECT_TRUE(std::dynamic_pointer_cast<display_device::NoopAudioContext>(settings_manager.getAudioContextApi()) != nullptr);
}

TEST_F_S(NullptrPersistentStateProvided) {
  EXPECT_THAT([this]() {
    const display_device::MacSettingsManager settings_manager(m_dd_api, nullptr, nullptr, {});
  },
              ThrowsMessage<std::invalid_argument>(HasSubstr("Nullptr provided for MacPersistentState in MacSettingsManager!")));
}

TEST_F_S(EnumAvailableDevices) {
  const display_device::EnumeratedDeviceList test_list {
    {"DeviceId1",
     "",
     "FriendlyName1",
     std::nullopt}
  };

  expectNoStateLoad();
  EXPECT_CALL(*m_dd_api, enumAvailableDevices())
    .Times(1)
    .WillOnce(Return(test_list));

  EXPECT_EQ(getImpl().enumAvailableDevices(), test_list);
}

TEST_F_S(GetDisplayName) {
  expectNoStateLoad();
  EXPECT_CALL(*m_dd_api, getDisplayName("DeviceId1"))
    .Times(1)
    .WillOnce(Return("DisplayName1"));

  EXPECT_EQ(getImpl().getDisplayName("DeviceId1"), "DisplayName1");
}

TEST_F_S(ResetPersistence, NoPersistence) {
  expectNoStateLoad();

  EXPECT_TRUE(getImpl().resetPersistence());
}

TEST_F_S(ResetPersistence, FailedToReset) {
  expectStoredStateLoad(makeState());
  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_FALSE(getImpl().resetPersistence());
}

TEST_F_S(ResetPersistence, PersistenceReset, NoCapturedDevice) {
  expectStoredStateLoad(makeState());
  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_audio_context_api, isCaptured())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_TRUE(getImpl().resetPersistence());
}

TEST_F_S(ResetPersistence, PersistenceReset, WithCapturedDevice) {
  expectStoredStateLoad(makeState());
  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_audio_context_api, isCaptured())
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_audio_context_api, release())
    .Times(1);

  EXPECT_TRUE(getImpl().resetPersistence());
}

TEST_F_S(ApplySettings, ApiTemporarilyUnavailable) {
  expectNoStateLoad();
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().applySettings({}), display_device::MacSettingsManager::ApplyResult::ApiTemporarilyUnavailable);
}

TEST_F_S(ApplySettings, HdrUnsupported) {
  expectNoStateLoad();
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_EQ(
    getImpl().applySettings({.m_hdr_state = display_device::HdrState::Enabled}),
    display_device::MacSettingsManager::ApplyResult::HdrStatePrepFailed
  );
}

TEST_F_S(ApplySettings, DisplayModeSuccess) {
  const auto expected_state {makeAppliedModeState()};

  expectNoStateLoad();
  Sequence sequence;
  expectApplyPreparation(sequence);
  expectCurrentModes(sequence, DEFAULT_MODES);
  expectSetModes(sequence, CHANGED_MODES, true);
  expectCurrentModes(sequence, CHANGED_MODES);
  expectStoreState(sequence, expected_state, true);

  EXPECT_EQ(
    getImpl().applySettings({.m_resolution = display_device::Resolution {1280, 720}}),
    display_device::MacSettingsManager::ApplyResult::Ok
  );
}

TEST_F_S(ApplySettings, DevicePrepUnsupported) {
  expectNoStateLoad();
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_EQ(
    getImpl().applySettings({.m_device_prep = display_device::SingleDisplayConfiguration::DevicePreparation::EnsureActive}),
    display_device::MacSettingsManager::ApplyResult::DevicePrepFailed
  );
}

TEST_F_S(ApplySettings, VerifyOnlyNoChanges) {
  const display_device::MacSingleDisplayConfigState expected_state {
    {DEFAULT_TOPOLOGY,
     {"DeviceId1"}},
    {display_device::MacSingleDisplayConfigState::Modified {
      DEFAULT_TOPOLOGY,
      {},
      {},
      {},
    }}
  };

  expectNoStateLoad();
  Sequence sequence;
  expectApplyPreparation(sequence);
  expectStoreState(sequence, expected_state, true);

  EXPECT_EQ(getImpl().applySettings({}), display_device::MacSettingsManager::ApplyResult::Ok);
}

TEST_F_S(ApplySettings, InactiveDevice) {
  expectNoStateLoad();
  Sequence sequence;
  expectApplyPreparation(sequence);

  EXPECT_EQ(
    getImpl().applySettings({.m_device_id = "DeviceId3"}),
    display_device::MacSettingsManager::ApplyResult::DevicePrepFailed
  );
}

TEST_F_S(ApplySettings, DisplayModeApplyFailed) {
  expectNoStateLoad();
  Sequence sequence;
  expectApplyPreparation(sequence);
  expectCurrentModes(sequence, DEFAULT_MODES);
  expectSetModes(sequence, CHANGED_MODES, false);

  EXPECT_EQ(
    getImpl().applySettings({.m_resolution = display_device::Resolution {1280, 720}}),
    display_device::MacSettingsManager::ApplyResult::DisplayModePrepFailed
  );
}

TEST_F_S(ApplySettings, PersistenceFailureRollsBackModes) {
  const auto expected_state {makeAppliedModeState()};

  expectNoStateLoad();
  Sequence sequence;
  expectApplyPreparation(sequence);
  expectCurrentModes(sequence, DEFAULT_MODES);
  expectSetModes(sequence, CHANGED_MODES, true);
  expectCurrentModes(sequence, CHANGED_MODES);
  expectStoreState(sequence, expected_state, false);
  expectSetModes(sequence, DEFAULT_MODES, true);

  EXPECT_EQ(
    getImpl().applySettings({.m_resolution = display_device::Resolution {1280, 720}}),
    display_device::MacSettingsManager::ApplyResult::PersistenceSaveFailed
  );
}

TEST_F_S(RevertSettings, NoPersistence) {
  expectNoStateLoad();

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::Ok);
}

TEST_F_S(RevertSettings, ApiTemporarilyUnavailable) {
  expectStoredStateLoad(makeState());
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::ApiTemporarilyUnavailable);
}

TEST_F_S(RevertSettings, ModeOnly) {
  expectStoredStateLoad(makeModeState());
  Sequence sequence;
  expectModeRevertPreparation(sequence, CURRENT_REVERT_MODES);
  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .InSequence(sequence)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_audio_context_api, isCaptured())
    .Times(1)
    .InSequence(sequence)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::Ok);
}

TEST_F_S(RevertSettings, PersistenceFailureRollsBackModes) {
  expectStoredStateLoad(makeModeState());
  Sequence sequence;
  expectModeRevertPreparation(sequence, CURRENT_REVERT_MODES);
  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .InSequence(sequence)
    .WillOnce(Return(false));
  expectSetModes(sequence, CURRENT_REVERT_MODES, true);

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::PersistenceSaveFailed);
}

TEST_F_S(RevertSettings, TopologyUnsupported) {
  auto state {makeModeState()};
  state.m_modified.m_topology = {{"DeviceId2"}};

  expectStoredStateLoad(state);
  InSequence sequence;
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_dd_api, getCurrentTopology())
    .Times(1)
    .WillOnce(Return(DEFAULT_TOPOLOGY));
  EXPECT_CALL(*m_dd_api, isTopologyValid(DEFAULT_TOPOLOGY))
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_dd_api, isTopologyValid(state.m_modified.m_topology))
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_dd_api, isTopologyTheSame(DEFAULT_TOPOLOGY, state.m_modified.m_topology))
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::SwitchingTopologyFailed);
}
