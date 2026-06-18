// local includes
#include "display_device/macos/json.h"
#include "display_device/macos/persistent_state.h"
#include "display_device/noop_settings_persistence.h"
#include "fixtures/fixtures.h"
#include "fixtures/mock_settings_persistence.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
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
  class MacPersistentStateMocked: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockSettingsPersistence>> m_settings_persistence_api {std::make_shared<StrictMock<display_device::MockSettingsPersistence>>()};
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, MacPersistentStateMocked, __VA_ARGS__)
}  // namespace

TEST_F_S(NoopPersistence) {
  const display_device::MacPersistentState persistent_state {nullptr};
  EXPECT_FALSE(persistent_state.getState());
  EXPECT_TRUE(std::dynamic_pointer_cast<display_device::NoopSettingsPersistence>(persistent_state.getSettingsPersistenceApi()) != nullptr);
}

TEST_F_S(LoadEmptyState) {
  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(std::optional<std::vector<std::uint8_t>> {std::vector<std::uint8_t> {}}));

  const display_device::MacPersistentState persistent_state {m_settings_persistence_api};
  EXPECT_FALSE(persistent_state.getState());
}

TEST_F_S(LoadStoredState) {
  const auto state {makeState()};

  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(state)));

  const display_device::MacPersistentState persistent_state {m_settings_persistence_api};
  EXPECT_EQ(persistent_state.getState(), state);
}

TEST_F_S(PersistState) {
  const auto state {makeState()};

  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(std::optional<std::vector<std::uint8_t>> {std::vector<std::uint8_t> {}}));

  display_device::MacPersistentState persistent_state {m_settings_persistence_api};

  EXPECT_CALL(*m_settings_persistence_api, store(_))
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_TRUE(persistent_state.persistState(state));
  EXPECT_EQ(persistent_state.getState(), state);
}

TEST_F_S(ClearState) {
  const auto state {makeState()};

  EXPECT_CALL(*m_settings_persistence_api, load())
    .Times(1)
    .WillOnce(Return(serializeState(state)));

  display_device::MacPersistentState persistent_state {m_settings_persistence_api};

  EXPECT_CALL(*m_settings_persistence_api, clear())
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_TRUE(persistent_state.persistState(std::nullopt));
  EXPECT_FALSE(persistent_state.getState());
}
