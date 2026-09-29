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
    bool split_clones {};
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
      if (split_clones && ids.contains("left") && ids.contains("right") && std::ranges::none_of(target, [](const auto &group) {
            return std::ranges::find(group, "left") != group.end() && std::ranges::find(group, "right") != group.end();
          })) {
        return false;  // The adapter cannot drive both members of the clone group from separate sources.
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

TEST_F(UndockRecovery, UnpluggedDisplayInModifiedTopologyDoesNotBlockRecovery) {
  const DisplayMode dock_mode {{3840, 2160}, {60, 1}};
  const DisplayMode stream_mode {{1920, 1080}, {60, 1}};
  state.m_modified = {{{"dock"}, {"stream"}}, {{"dock", dock_mode}, {"stream", stream_mode}}, {{"dock", HdrState::Enabled}, {"stream", HdrState::Disabled}}, "dock"};
  std::vector<DeviceDisplayModeMap> mode_writes;
  std::vector<HdrStateMap> hdr_writes;
  init();
  ON_CALL(*api, setDisplayModes(_)).WillByDefault([&mode_writes](const DeviceDisplayModeMap &modes) {
    mode_writes.push_back(modes);
    return true;
  });
  ON_CALL(*api, setHdrStates(_)).WillByDefault([&hdr_writes](const HdrStateMap &states) {
    hdr_writes.push_back(states);
    return true;
  });
  EXPECT_CALL(*api, setAsPrimary(_)).Times(0);

  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::SwitchingTopologyFailed);
  EXPECT_EQ(clears, 0);
  EXPECT_EQ(active, (ActiveTopology {{"stream"}}));
  ASSERT_FALSE(mode_writes.empty());
  EXPECT_EQ(mode_writes.front(), (DeviceDisplayModeMap {{"stream", stream_mode}}));
  ASSERT_FALSE(hdr_writes.empty());
  EXPECT_EQ(hdr_writes.front(), (HdrStateMap {{"stream", HdrState::Disabled}}));

  openLid();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"panel"}}));
  EXPECT_EQ(clears, 0);  // The dock display still has settings to restore.

  // Redocking restores what the unplugged display had before the stream.
  devices.push_back({.m_device_id = "dock"});
  mode_writes.clear();
  hdr_writes.clear();
  ::testing::Mock::VerifyAndClearExpectations(api.get());
  EXPECT_CALL(*api, setAsPrimary("dock")).WillOnce(Return(true));
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  ASSERT_FALSE(mode_writes.empty());
  EXPECT_EQ(mode_writes.front(), (DeviceDisplayModeMap {{"dock", dock_mode}}));
  ASSERT_FALSE(hdr_writes.empty());
  EXPECT_EQ(hdr_writes.front(), (HdrStateMap {{"dock", HdrState::Enabled}}));
  EXPECT_EQ(win_utils::flattenTopology(active), (StringSet {"dock", "panel"}));
  EXPECT_EQ(clears, 1);
}

TEST_F(UndockRecovery, DoesNotRetainSettingsOfStreamOnlyDisplay) {
  const DisplayMode stream_mode {{1920, 1080}, {60, 1}};
  state.m_modified = {{{"stream"}}, {{"stream", stream_mode}}, {}, {}};
  openLid();
  init();
  ON_CALL(*api, setDisplayModes(_)).WillByDefault(Return(true));
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"panel"}}));
  EXPECT_EQ(clears, 1);
}

TEST_F(UndockRecovery, PendingSettingsSurviveApplyWhileStillUndocked) {
  const DisplayMode dock_mode {{3840, 2160}, {60, 1}};
  state.m_initial = {{{"dock"}, {"panel"}}, {"dock"}};  // As retained after a recovery to the panel.
  state.m_modified = {{{"dock"}}, {{"dock", dock_mode}}, {{"dock", HdrState::Enabled}}, "dock"};
  active = {{"panel"}};
  devices = {{.m_device_id = "panel", .m_info = EnumeratedDevice::Info {.m_primary = true}, .m_is_internal = true}};
  std::vector<DeviceDisplayModeMap> mode_writes;
  init();
  ON_CALL(*api, setDisplayModes(_)).WillByDefault([&mode_writes](const DeviceDisplayModeMap &modes) {
    mode_writes.push_back(modes);
    return true;
  });
  ON_CALL(*api, setHdrStates(_)).WillByDefault(Return(true));
  ON_CALL(*api, setAsPrimary(_)).WillByDefault(Return(true));
  ON_CALL(*api, isPrimary("panel")).WillByDefault(Return(true));
  ON_CALL(*api, getCurrentDisplayModes(_)).WillByDefault(Return(DeviceDisplayModeMap {{"panel", {{1920, 1080}, {60, 1}}}}));
  ON_CALL(*api, getCurrentHdrStates(_)).WillByDefault(Return(HdrStateMap {{"panel", HdrState::Disabled}}));

  SingleDisplayConfiguration config;
  config.m_device_id = "panel";
  config.m_device_prep = SingleDisplayConfiguration::DevicePreparation::VerifyOnly;
  EXPECT_CALL(*api, setAsPrimary("dock")).Times(0);
  EXPECT_EQ(manager->applySettings(config), SettingsManager::ApplyResult::Ok);
  EXPECT_TRUE(mode_writes.empty());

  // The dock returns and gets its own settings back.
  devices.push_back({.m_device_id = "dock"});
  ::testing::Mock::VerifyAndClearExpectations(api.get());
  EXPECT_CALL(*api, setAsPrimary("dock")).WillOnce(Return(true));
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  ASSERT_FALSE(mode_writes.empty());
  EXPECT_EQ(mode_writes.front(), (DeviceDisplayModeMap {{"dock", dock_mode}}));
  EXPECT_EQ(clears, 1);
}

TEST_F(UndockRecovery, EnsurePrimaryWhileOriginalPrimaryIsAway) {
  // The dock was primary. After it left, "A" took over and the dock's primary status is pending.
  state.m_initial = {{{"dock"}, {"A"}, {"B"}}, {"dock"}};
  state.m_modified = {{{"dock"}}, {}, {}, "dock"};
  active = {{"A"}, {"B"}};
  devices = {
    {.m_device_id = "A", .m_info = EnumeratedDevice::Info {.m_primary = true}},
    {.m_device_id = "B", .m_info = EnumeratedDevice::Info {}},
  };
  std::string primary {"A"};
  init();
  ON_CALL(*api, isPrimary(_)).WillByDefault([&primary](const std::string &id) {
    return id == primary;
  });
  ON_CALL(*api, setAsPrimary(_)).WillByDefault([&primary](const std::string &id) {
    primary = id;
    return true;
  });

  SingleDisplayConfiguration config;
  config.m_device_id = "B";
  config.m_device_prep = SingleDisplayConfiguration::DevicePreparation::EnsurePrimary;
  EXPECT_EQ(manager->applySettings(config), SettingsManager::ApplyResult::Ok);
  EXPECT_EQ(primary, "B");

  // Reverting while the dock is still away brings back the display that was primary before the session.
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(primary, "A");
  EXPECT_EQ(clears, 0);

  // The dock's own primary status is still pending and is restored once it returns.
  devices.push_back({.m_device_id = "dock"});
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(primary, "dock");
  EXPECT_EQ(clears, 1);
}

TEST_F(UndockRecovery, StagingKeepsCloneGroupWithActiveMember) {
  state.m_initial.m_topology.push_back({"left", "right"});
  devices.push_back({.m_device_id = "left"});
  devices.push_back({.m_device_id = "right"});
  active = {{"left"}, {"stream"}};
  split_clones = true;
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"left", "right"}}));
  ASSERT_GE(writes.size(), 3u);
  EXPECT_EQ(writes[writes.size() - 2], (ActiveTopology {{"left", "right"}, {"stream"}}));
}

TEST_F(UndockRecovery, StagingKeepsSurvivingCloneGroup) {
  state.m_initial.m_topology.push_back({"left", "right"});
  devices.push_back({.m_device_id = "left"});
  devices.push_back({.m_device_id = "right"});
  split_clones = true;
  init();
  EXPECT_EQ(manager->revertSettings(), SettingsManager::RevertResult::Ok);
  EXPECT_EQ(active, (ActiveTopology {{"left", "right"}}));
  ASSERT_GE(writes.size(), 3u);
  EXPECT_EQ(writes[writes.size() - 2], (ActiveTopology {{"stream"}, {"left", "right"}}));
}
