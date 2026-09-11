/**
 * @file tests/unit/windows/test_settings_manager_undock.cpp
 * @brief Tests for display recovery across laptop lid and docking changes.
 */
#include "display_device/windows/settings_manager.h"
#include "display_device/windows/settings_utils.h"
#include "fixtures/fixtures.h"
#include "fixtures/mock_audio_context.h"
#include "fixtures/mock_settings_persistence.h"
#include "utils/helpers.h"
#include "utils/mock_win_display_device.h"

#include <algorithm>

namespace {
  using namespace display_device;
  using ::testing::_;
  using ::testing::NiceMock;
  using ::testing::Return;

  class UndockRecovery: public BaseTest {
  public:
    ActiveTopology active {{"stream"}};
    EnumeratedDeviceList devices {{.m_device_id = "stream"}};
    SingleDisplayConfigState state {{{{"dock"}}, {"dock"}}, {{{"stream"}}, {}, {}, {}}};
    std::vector<ActiveTopology> writes;
    bool fail_activation {};
    bool lie_about_activation {};
    bool fail_cleanup {};
    bool fail_clear {};
    bool lie_about_cleanup {};
    int clears {};
    std::shared_ptr<NiceMock<MockWinDisplayDevice>> api = std::make_shared<NiceMock<MockWinDisplayDevice>>();
    std::shared_ptr<NiceMock<MockSettingsPersistence>> persistence = std::make_shared<NiceMock<MockSettingsPersistence>>();
    std::shared_ptr<NiceMock<MockAudioContext>> audio = std::make_shared<NiceMock<MockAudioContext>>();
    std::unique_ptr<SettingsManager> manager;

    /**
     * @brief Simulate topology writes, including unavailable devices and unreliable API results.
     * @param target Requested active display groups.
     * @return Whether the simulated API reports success.
     */
    bool applyTopology(const ActiveTopology &target) {
      writes.push_back(target);
      const auto ids = win_utils::flattenTopology(target);
      if (ids.empty()) {
        ADD_FAILURE() << "Attempted to blank every output";
        return false;
      }
      for (const auto &id : ids) {
        if (std::ranges::none_of(devices, [&id](const auto &d) {
              return d.m_device_id == id;
            })) {
          return false;
        }
      }
      if (fail_activation && ids.contains("panel")) {
        return false;
      }
      if (fail_cleanup && !ids.contains("stream")) {
        return false;
      }
      if (!lie_about_activation && !(lie_about_cleanup && !ids.contains("stream"))) {
        active = target;
      }
      return true;
    }

    /** @brief Connect the stateful display and persistence mocks to the manager. */
    void init() {
      ON_CALL(*api, isApiAccessAvailable()).WillByDefault(Return(true));
      ON_CALL(*api, enumAvailableDevices()).WillByDefault([this] {
        return devices;
      });
      ON_CALL(*api, getCurrentTopology()).WillByDefault([this] {
        return active;
      });
      ON_CALL(*api, isTopologyValid(_)).WillByDefault([](const auto &t) {
        return !t.empty();
      });
      ON_CALL(*api, isTopologyTheSame(_, _)).WillByDefault([](const auto &a, const auto &b) {
        return win_utils::flattenTopology(a) == win_utils::flattenTopology(b);
      });
      ON_CALL(*api, setTopology(_)).WillByDefault([this](const ActiveTopology &target) {
        return applyTopology(target);
      });
      ON_CALL(*persistence, load()).WillByDefault([this] {
        return serializeState(state);
      });
      ON_CALL(*persistence, store(_)).WillByDefault(Return(true));
      ON_CALL(*persistence, clear()).WillByDefault([this] {
        if (fail_clear) {
          return false;
        }
        ++clears;
        return true;
      });
      manager = std::make_unique<SettingsManager>(api, audio, std::make_unique<PersistentState>(persistence), WinWorkarounds {});
    }

    /** @brief Make the built-in panel available after opening the lid. */
    void openLid() {
      devices.push_back({.m_device_id = "panel", .m_is_internal = true});
    }
  };
}  // namespace

TEST_F(UndockRecovery, ClosedLidRetainsRecoveryUntilPanelAppears) {
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  EXPECT_EQ(clears, 0);
  EXPECT_EQ(active, (ActiveTopology {{"stream"}}));
  openLid();
  writes.clear();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"panel"}}));
  ASSERT_GE(writes.size(), 3u);
  EXPECT_EQ(win_utils::flattenTopology(writes[writes.size() - 2]), (StringSet {"panel", "stream"}));
  EXPECT_EQ(clears, 1);
  writes.clear();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_TRUE(writes.empty());
}

TEST_F(UndockRecovery, RecoverySurvivesControllerRestart) {
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  manager.reset();
  openLid();
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"panel"}}));
}

TEST_F(UndockRecovery, RedockRestoresOriginalOutput) {
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  devices.push_back({.m_device_id = "dock"});
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"dock"}}));
  EXPECT_EQ(clears, 1);
}

TEST_F(UndockRecovery, PreservesUnrelatedActiveOutput) {
  openLid();
  devices.push_back({.m_device_id = "other"});
  active.push_back({"other"});
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(win_utils::flattenTopology(active), (StringSet {"panel", "other"}));
}

TEST_F(UndockRecovery, RestoresSurvivingOriginalDisplay) {
  state.m_initial.m_topology.push_back({"survivor"});
  devices.push_back({.m_device_id = "survivor"});
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"survivor"}}));
}

TEST_F(UndockRecovery, DoesNotMistakeUnknownOutputForLaptopPanel) {
  devices.push_back({.m_device_id = "unknown"});
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  EXPECT_EQ(clears, 0);
}

TEST_F(UndockRecovery, ActivationFailureDoesNotRemoveStreamingOutput) {
  openLid();
  fail_activation = true;
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  EXPECT_EQ(clears, 0);
  EXPECT_TRUE(win_utils::flattenTopology(active).contains("stream"));
}

TEST_F(UndockRecovery, ActivationRequiresReadback) {
  openLid();
  lie_about_activation = true;
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  EXPECT_EQ(clears, 0);
  for (const auto &target : writes) {
    if (!win_utils::flattenTopology(target).contains("dock")) {
      EXPECT_TRUE(win_utils::flattenTopology(target).contains("stream"));
    }
  }
}

TEST_F(UndockRecovery, CleanupFailureRetainsPersistence) {
  openLid();
  fail_cleanup = true;
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  EXPECT_EQ(clears, 0);
  EXPECT_TRUE(win_utils::flattenTopology(active).contains("panel"));
}

TEST_F(UndockRecovery, FailedPersistenceCanRetry) {
  openLid();
  fail_clear = true;
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::PersistenceSaveFailed);
  EXPECT_EQ(clears, 0);
  fail_clear = false;
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"panel"}}));
  EXPECT_EQ(clears, 1);
}

TEST_F(UndockRecovery, CleanupRequiresReadback) {
  openLid();
  lie_about_cleanup = true;
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  EXPECT_EQ(clears, 0);
  EXPECT_TRUE(win_utils::flattenTopology(active).contains("stream"));
}
