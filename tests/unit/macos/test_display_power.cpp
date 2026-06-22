// system includes
#include <chrono>
#include <gmock/gmock.h>
#include <stdexcept>

// local includes
#include "display_device/macos/display_power.h"
#include "fixtures/fixtures.h"
#include "utils/mock_mac_api_layer.h"

namespace {
  using namespace std::chrono_literals;

  // Convenience keywords for GMock
  using ::testing::HasSubstr;
  using ::testing::InSequence;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class MacDisplayPowerTest: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockMacApiLayer>> m_layer {std::make_shared<StrictMock<display_device::MockMacApiLayer>>()};
    display_device::MacDisplayPower m_power {m_layer};
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, MacDisplayPowerTest, __VA_ARGS__)
}  // namespace

TEST_F_S(NullptrLayerProvided) {
  EXPECT_THAT([]() {
    const auto power {display_device::MacDisplayPower {nullptr}};
  },
              ThrowsMessage<std::invalid_argument>(HasSubstr("Nullptr provided for MacApiLayerInterface in MacDisplayPower!")));
}

TEST_F_S(KeepDisplayAwakeCreatesAndReleasesAssertion) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, createDisplaySleepAssertion("Test capture"))
    .Times(1)
    .WillOnce(Return(display_device::MacPowerAssertionId {42}));
  EXPECT_CALL(*m_layer, releasePowerAssertion(42))
    .Times(1)
    .WillOnce(Return(true));

  {
    const auto guard {m_power.keepDisplayAwake("Test capture")};
    ASSERT_NE(guard, nullptr);
  }
}

TEST_F_S(KeepDisplayAwakeReturnsNullptrWhenAssertionFails) {
  EXPECT_CALL(*m_layer, createDisplaySleepAssertion("Test capture"))
    .Times(1)
    .WillOnce(Return(std::nullopt));

  EXPECT_EQ(m_power.keepDisplayAwake("Test capture"), nullptr);
}

TEST_F_S(WakeDisplayReturnsTrueWhenRequestedDisplayIsAlreadyActive) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {42}));

  EXPECT_TRUE(m_power.wakeDisplay("42", 0ms));
}

TEST_F_S(WakeDisplayUsesAnyActiveDisplayForNonNumericSelector) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {42}));

  EXPECT_TRUE(m_power.wakeDisplay("Built-in Display", 0ms));
}

TEST_F_S(WakeDisplayDeclaresUserActivityAndReleasesAssertion) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {}));
  EXPECT_CALL(*m_layer, declareUserActivity("libdisplaydevice display detection"))
    .Times(1)
    .WillOnce(Return(display_device::MacPowerAssertionId {7}));
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {42}));
  EXPECT_CALL(*m_layer, releasePowerAssertion(7))
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_TRUE(m_power.wakeDisplay("42", 0ms));
}

TEST_F_S(WakeDisplayReturnsFalseWhenAssertionFails) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {}));
  EXPECT_CALL(*m_layer, declareUserActivity("libdisplaydevice display detection"))
    .Times(1)
    .WillOnce(Return(std::nullopt));

  EXPECT_FALSE(m_power.wakeDisplay("42", 0ms));
}

TEST_F_S(WakeDisplayReturnsFalseAfterTimeoutAndReleasesAssertion) {
  InSequence sequence;
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {}));
  EXPECT_CALL(*m_layer, declareUserActivity("libdisplaydevice display detection"))
    .Times(1)
    .WillOnce(Return(display_device::MacPowerAssertionId {7}));
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(2)
    .WillRepeatedly(Return(display_device::MacDisplayIdList {}));
  EXPECT_CALL(*m_layer, releasePowerAssertion(7))
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_FALSE(m_power.wakeDisplay("42", 0ms));
}
