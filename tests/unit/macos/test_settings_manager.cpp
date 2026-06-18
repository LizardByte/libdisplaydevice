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
  using ::testing::Return;
  using ::testing::StrictMock;

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

  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));
  EXPECT_CALL(*m_dd_api, enumAvailableDevices())
    .Times(1)
    .WillOnce(Return(test_list));

  EXPECT_EQ(getImpl().enumAvailableDevices(), test_list);
}

TEST_F_S(GetDisplayName) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));
  EXPECT_CALL(*m_dd_api, getDisplayName("DeviceId1"))
    .Times(1)
    .WillOnce(Return("DisplayName1"));

  EXPECT_EQ(getImpl().getDisplayName("DeviceId1"), "DisplayName1");
}

TEST_F_S(ResetPersistence, NoPersistence) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));

  EXPECT_TRUE(getImpl().resetPersistence());
}

TEST_F_S(ResetPersistence, FailedToReset) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(makeState())));
  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_FALSE(getImpl().resetPersistence());
}

TEST_F_S(ResetPersistence, PersistenceReset, NoCapturedDevice) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(makeState())));
  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_audio_context_api, isCaptured())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_TRUE(getImpl().resetPersistence());
}

TEST_F_S(ResetPersistence, PersistenceReset, WithCapturedDevice) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(makeState())));
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
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().applySettings({}), display_device::MacSettingsManager::ApplyResult::ApiTemporarilyUnavailable);
}

TEST_F_S(ApplySettings, HdrUnsupported) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_EQ(
    getImpl().applySettings({.m_hdr_state = display_device::HdrState::Enabled}),
    display_device::MacSettingsManager::ApplyResult::HdrStatePrepFailed
  );
}

TEST_F_S(ApplySettings, DisplayModeUnsupported) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_EQ(
    getImpl().applySettings({.m_resolution = display_device::Resolution {1920, 1080}}),
    display_device::MacSettingsManager::ApplyResult::DisplayModePrepFailed
  );
}

TEST_F_S(ApplySettings, DevicePrepUnsupported) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_EQ(getImpl().applySettings({}), display_device::MacSettingsManager::ApplyResult::DevicePrepFailed);
}

TEST_F_S(RevertSettings, NoPersistence) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeNoState()));

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::Ok);
}

TEST_F_S(RevertSettings, ApiTemporarilyUnavailable) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(makeState())));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::ApiTemporarilyUnavailable);
}

TEST_F_S(RevertSettings, TopologyUnsupported) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(makeState())));
  EXPECT_CALL(*m_dd_api, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_EQ(getImpl().revertSettings(), display_device::MacSettingsManager::RevertResult::SwitchingTopologyFailed);
}
