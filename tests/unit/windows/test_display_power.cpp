// system includes
#include <chrono>
#include <gmock/gmock.h>
#include <stdexcept>

// local includes
#include "display_device/windows/display_power.h"
#include "fixtures/fixtures.h"
#include "utils/mock_win_api_layer.h"

namespace {
  using namespace std::chrono_literals;

  // Convenience keywords for GMock
  using ::testing::HasSubstr;
  using ::testing::InSequence;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class WinDisplayPowerTest: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockWinApiLayer>> m_layer {std::make_shared<StrictMock<display_device::MockWinApiLayer>>()};
    display_device::WinDisplayPower m_power {m_layer};
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, WinDisplayPowerTest, __VA_ARGS__)
}  // namespace

TEST_F_S(NullptrLayerProvided) {
  EXPECT_THAT([]() {
    const auto power {display_device::WinDisplayPower {nullptr}};
  },
              ThrowsMessage<std::invalid_argument>(HasSubstr("Nullptr provided for WinApiLayerInterface in WinDisplayPower!")));
}

TEST_F_S(WakeDisplayForwardsTimeout) {
  EXPECT_CALL(*m_layer, wakeDisplay(250ms))
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_TRUE(m_power.wakeDisplay("\\\\.\\DISPLAY1", 250ms));
}

TEST_F_S(WakeDisplayReturnsFalseWhenApiFails) {
  EXPECT_CALL(*m_layer, wakeDisplay(250ms))
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_FALSE(m_power.wakeDisplay("\\\\.\\DISPLAY1", 250ms));
}

TEST_F_S(KeepDisplayAwakeCreatesAndRestoresRequest) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, keepDisplayAwake())
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_layer, restorePowerRequest())
    .Times(1)
    .WillOnce(Return(true));

  {
    const auto guard {m_power.keepDisplayAwake("Test capture")};
    ASSERT_NE(guard, nullptr);
  }
}

TEST_F_S(KeepDisplayAwakeReturnsNullptrWhenApiFails) {
  EXPECT_CALL(*m_layer, keepDisplayAwake())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_EQ(m_power.keepDisplayAwake("Test capture"), nullptr);
}
