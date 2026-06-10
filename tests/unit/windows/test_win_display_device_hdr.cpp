// local includes
#include "display_device/windows/settings_utils.h"
#include "display_device/windows/win_api_layer.h"
#include "display_device/windows/win_display_device.h"
#include "fixtures/fixtures.h"
#include "utils/comparison.h"
#include "utils/guards.h"
#include "utils/helpers.h"
#include "utils/mock_win_api_layer.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::InSequence;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class WinDisplayDeviceHdr: public BaseTest {
  public:
    bool isSystemTest() const override {
      return true;
    }

    std::shared_ptr<display_device::WinApiLayer> m_layer {std::make_shared<display_device::WinApiLayer>()};
    display_device::WinDisplayDevice m_win_dd {m_layer};
  };

  class WinDisplayDeviceHdrMocked: public BaseTest {
  public:
    void setupExpectedGetActivePathCall(int id_number, InSequence & /* To ensure that sequence is created outside this scope */) const {
      expectActivePathLookup(m_layer, id_number);
    }

    void setupExpectedDuplicatedPathHdrStateRead(
      const int id_number,
      const int path_index,
      const std::optional<display_device::HdrState> &state,
      InSequence &sequence
    ) const {
      setupExpectedGetActivePathCall(id_number, sequence);
      EXPECT_CALL(*m_layer, getHdrState(ut_consts::PAM_4_ACTIVE_WITH_2_DUPLICATES->m_paths.at(path_index)))
        .Times(1)
        .WillOnce(Return(state))
        .RetiresOnSaturation();
    }

    void setupExpectedDuplicatedPathHdrStateChange(
      const int id_number,
      const int path_index,
      const std::optional<display_device::HdrState> &current_state,
      const display_device::HdrState new_state,
      const bool result,
      InSequence &sequence
    ) const {
      setupExpectedDuplicatedPathHdrStateRead(id_number, path_index, current_state, sequence);
      EXPECT_CALL(*m_layer, setHdrState(ut_consts::PAM_4_ACTIVE_WITH_2_DUPLICATES->m_paths.at(path_index), new_state))
        .Times(1)
        .WillOnce(Return(result))
        .RetiresOnSaturation();
    }

    std::shared_ptr<StrictMock<display_device::MockWinApiLayer>> m_layer {std::make_shared<StrictMock<display_device::MockWinApiLayer>>()};
    display_device::WinDisplayDevice m_win_dd {m_layer};
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, WinDisplayDeviceHdr, __VA_ARGS__)
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, WinDisplayDeviceHdrMocked, __VA_ARGS__)

  // Helper functions
  display_device::ActiveTopology makeExtendedTopology(const std::vector<std::string> &device_ids) {
    display_device::ActiveTopology topology;
    for (const auto &device_id : device_ids) {
      topology.push_back({device_id});
    }
    return topology;
  }
}  // namespace

TEST_F_S(GetSetHdrStates) {
  const auto available_devices {getAvailableDevices(*m_layer, false)};
  ASSERT_TRUE(available_devices);

  if (available_devices->empty()) {
    GTEST_SKIP_("No displays are available in the system.");
  }

  const auto topology_guard {makeTopologyGuard(m_win_dd)};
  ASSERT_TRUE(m_win_dd.setTopology(makeExtendedTopology(*available_devices)));

  const auto hdr_states {m_win_dd.getCurrentHdrStates(display_device::win_utils::flattenTopology(m_win_dd.getCurrentTopology()))};
  if (!std::ranges::any_of(hdr_states, [](auto entry) {
        return static_cast<bool>(entry.second);
      })) {
    GTEST_SKIP_("No HDR display is available in the system.");
  }

  auto flipped_states {hdr_states};
  for (auto &[key, state] : flipped_states) {
    if (state) {
      state = *state == display_device::HdrState::Disabled ?
                display_device::HdrState::Enabled :
                display_device::HdrState::Disabled;
    }
  }

  const auto hdr_state_guard {makeHdrStateGuard(m_win_dd)};
  ASSERT_TRUE(m_win_dd.setHdrStates(flipped_states));
  ASSERT_TRUE(m_win_dd.setHdrStates(hdr_states));
}

TEST_F_S_MOCKED(GetHdrStates) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE))
    .RetiresOnSaturation();

  setupExpectedGetActivePathCall(1, sequence);
  EXPECT_CALL(*m_layer, getHdrState(ut_consts::PAM_3_ACTIVE->m_paths.at(0)))
    .Times(1)
    .WillOnce(Return(std::make_optional(display_device::HdrState::Disabled)))
    .RetiresOnSaturation();

  setupExpectedGetActivePathCall(2, sequence);
  EXPECT_CALL(*m_layer, getHdrState(ut_consts::PAM_3_ACTIVE->m_paths.at(1)))
    .Times(1)
    .WillOnce(Return(std::nullopt))
    .RetiresOnSaturation();

  setupExpectedGetActivePathCall(3, sequence);
  EXPECT_CALL(*m_layer, getHdrState(ut_consts::PAM_3_ACTIVE->m_paths.at(2)))
    .Times(1)
    .WillOnce(Return(std::make_optional(display_device::HdrState::Enabled)))
    .RetiresOnSaturation();

  const display_device::HdrStateMap expected_states {
    {"DeviceId1", std::make_optional(display_device::HdrState::Disabled)},
    {"DeviceId2", std::nullopt},
    {"DeviceId3", std::make_optional(display_device::HdrState::Enabled)}
  };
  EXPECT_EQ(m_win_dd.getCurrentHdrStates({"DeviceId1", "DeviceId2", "DeviceId3"}), expected_states);
}

TEST_F_S_MOCKED(GetHdrStates, EmptyIdList) {
  EXPECT_EQ(m_win_dd.getCurrentHdrStates({}), display_device::HdrStateMap {});
}

TEST_F_S_MOCKED(GetHdrStates, FailedToGetDisplayData) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_NULL));

  EXPECT_EQ(m_win_dd.getCurrentHdrStates({"DeviceId1"}), display_device::HdrStateMap {});
}

TEST_F_S_MOCKED(GetHdrStates, FailedToGetActivePath) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_EMPTY));

  EXPECT_EQ(m_win_dd.getCurrentHdrStates({"DeviceId1"}), display_device::HdrStateMap {});
}

TEST_F_S_MOCKED(SetHdrStates) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE))
    .RetiresOnSaturation();

  setupExpectedGetActivePathCall(1, sequence);
  EXPECT_CALL(*m_layer, getHdrState(ut_consts::PAM_3_ACTIVE->m_paths.at(0)))
    .Times(1)
    .WillOnce(Return(std::make_optional(display_device::HdrState::Disabled)))
    .RetiresOnSaturation();
  EXPECT_CALL(*m_layer, setHdrState(ut_consts::PAM_3_ACTIVE->m_paths.at(0), display_device::HdrState::Enabled))
    .Times(1)
    .WillOnce(Return(true))
    .RetiresOnSaturation();

  setupExpectedGetActivePathCall(3, sequence);
  EXPECT_CALL(*m_layer, getHdrState(ut_consts::PAM_3_ACTIVE->m_paths.at(2)))
    .Times(1)
    .WillOnce(Return(std::make_optional(display_device::HdrState::Enabled)))
    .RetiresOnSaturation();

  const display_device::HdrStateMap new_states {
    {"DeviceId1", std::make_optional(display_device::HdrState::Enabled)},
    {"DeviceId2", std::nullopt},
    {"DeviceId3", std::make_optional(display_device::HdrState::Enabled)}
  };
  EXPECT_TRUE(m_win_dd.setHdrStates(new_states));
}

TEST_F_S_MOCKED(SetHdrStates, AllDevicesWithOptStates) {
  const display_device::HdrStateMap new_states {
    {"DeviceId1", std::nullopt},
    {"DeviceId2", std::nullopt},
    {"DeviceId3", std::nullopt}
  };
  EXPECT_TRUE(m_win_dd.setHdrStates(new_states));
}

TEST_F_S_MOCKED(SetHdrStates, FailedToGetHdrState) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_3_ACTIVE))
    .RetiresOnSaturation();

  setupExpectedGetActivePathCall(1, sequence);
  EXPECT_CALL(*m_layer, getHdrState(ut_consts::PAM_3_ACTIVE->m_paths.at(0)))
    .Times(1)
    .WillOnce(Return(std::nullopt))
    .RetiresOnSaturation();

  const display_device::HdrStateMap new_states {
    {"DeviceId1", std::make_optional(display_device::HdrState::Enabled)},
    {"DeviceId2", std::nullopt},
    {"DeviceId3", std::make_optional(display_device::HdrState::Enabled)}
  };
  EXPECT_FALSE(m_win_dd.setHdrStates(new_states));
}

TEST_F_S_MOCKED(SetHdrStates, FailedToSetHdrState, LastDevice) {
  using enum display_device::HdrState;

  InSequence sequence;
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_4_ACTIVE_WITH_2_DUPLICATES))
    .RetiresOnSaturation();

  // Setting states
  {
    setupExpectedDuplicatedPathHdrStateChange(1, 0, Disabled, Enabled, true, sequence);
    setupExpectedDuplicatedPathHdrStateRead(3, 2, Disabled, sequence);
    setupExpectedDuplicatedPathHdrStateRead(4, 3, std::nullopt, sequence);
  }

  // Reverting only changed states
  {
    setupExpectedDuplicatedPathHdrStateChange(1, 0, Enabled, Disabled, true, sequence);
  }

  const display_device::HdrStateMap new_states {
    {"DeviceId1", std::make_optional(Enabled)},
    {"DeviceId2", std::nullopt},
    {"DeviceId3", std::make_optional(Disabled)},
    {"DeviceId4", std::make_optional(Enabled)}
  };
  EXPECT_FALSE(m_win_dd.setHdrStates(new_states));
}

TEST_F_S_MOCKED(SetHdrStates, FailedToSetHdrState, LastDevice, NoEarlyExitInRecovery) {
  using enum display_device::HdrState;

  InSequence sequence;
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_4_ACTIVE_WITH_2_DUPLICATES))
    .RetiresOnSaturation();

  // Setting states
  {
    setupExpectedDuplicatedPathHdrStateChange(1, 0, Disabled, Enabled, true, sequence);
    setupExpectedDuplicatedPathHdrStateChange(2, 1, Disabled, Enabled, true, sequence);
    setupExpectedDuplicatedPathHdrStateChange(3, 2, Disabled, Enabled, true, sequence);
    setupExpectedDuplicatedPathHdrStateRead(4, 3, std::nullopt, sequence);
  }

  // Reverting only changed states
  {
    setupExpectedDuplicatedPathHdrStateChange(1, 0, Enabled, Disabled, false, sequence);
    setupExpectedDuplicatedPathHdrStateRead(2, 1, std::nullopt, sequence);
    setupExpectedDuplicatedPathHdrStateChange(3, 2, Enabled, Disabled, true, sequence);
  }

  const display_device::HdrStateMap new_states {
    {"DeviceId1", std::make_optional(Enabled)},
    {"DeviceId2", std::make_optional(Enabled)},
    {"DeviceId3", std::make_optional(Enabled)},
    {"DeviceId4", std::make_optional(Enabled)}
  };
  EXPECT_FALSE(m_win_dd.setHdrStates(new_states));
}

TEST_F_S_MOCKED(SetHdrStates, EmptyMap) {
  EXPECT_FALSE(m_win_dd.setHdrStates({}));
}

TEST_F_S_MOCKED(SetHdrStates, FailedToGetDisplayData) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_NULL));

  EXPECT_FALSE(m_win_dd.setHdrStates({{"DeviceId1", std::make_optional(display_device::HdrState::Disabled)}}));
}

TEST_F_S_MOCKED(SetHdrStates, FailedToGetActivePath) {
  EXPECT_CALL(*m_layer, queryDisplayConfig(display_device::QueryType::Active))
    .Times(1)
    .WillOnce(Return(ut_consts::PAM_EMPTY));

  EXPECT_FALSE(m_win_dd.setHdrStates({{"DeviceId1", std::make_optional(display_device::HdrState::Disabled)}}));
}
