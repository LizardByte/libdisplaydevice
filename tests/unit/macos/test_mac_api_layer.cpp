// system includes
#include <algorithm>
#include <gmock/gmock.h>
#include <string>

// local includes
#include "display_device/macos/mac_api_layer.h"
#include "fixtures/fixtures.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::HasSubstr;

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, MacApiLayer, __VA_ARGS__)

  // Test fixture(s) for this file
  class MacApiLayer: public BaseTest {
  public:
    display_device::MacApiLayer m_layer;
  };
}  // namespace

TEST_F_S(IsApiAccessAvailable) {
  EXPECT_TRUE(m_layer.isApiAccessAvailable());
}

TEST_F_S(GetErrorString) {
  EXPECT_THAT(m_layer.getErrorString(0), HasSubstr("Success"));
  EXPECT_THAT(m_layer.getErrorString(7), HasSubstr("Unknown CoreGraphics error"));
}

TEST_F_S(GetDisplayIds) {
  const auto active_displays {m_layer.getDisplayIds(display_device::MacQueryType::Active)};
  const auto online_displays {m_layer.getDisplayIds(display_device::MacQueryType::Online)};

  ASSERT_FALSE(active_displays.empty());
  EXPECT_GE(online_displays.size(), active_displays.size());
}

TEST_F_S(QueryActiveDisplay) {
  const auto active_displays {m_layer.getDisplayIds(display_device::MacQueryType::Active)};
  ASSERT_FALSE(active_displays.empty());

  const auto display_id {active_displays.front()};
  EXPECT_FALSE(m_layer.getDeviceId(display_id).empty());
  EXPECT_EQ(m_layer.getDisplayName(display_id), std::to_string(display_id));
  EXPECT_TRUE(m_layer.isActive(display_id));
  EXPECT_TRUE(m_layer.isOnline(display_id));

  const auto current_mode {m_layer.getCurrentDisplayMode(display_id)};
  ASSERT_TRUE(current_mode);
  EXPECT_GT(current_mode->m_resolution.m_width, 0U);
  EXPECT_GT(current_mode->m_resolution.m_height, 0U);
  EXPECT_GT(current_mode->m_refresh_rate.m_denominator, 0U);

  const auto origin {m_layer.getOriginPoint(display_id)};
  EXPECT_TRUE(origin);

  const auto modes {m_layer.getDisplayModes(display_id)};
  EXPECT_TRUE(std::ranges::find(modes, *current_mode) != std::end(modes) || !modes.empty());
}

TEST_F_S(HasMainDisplay) {
  const auto active_displays {m_layer.getDisplayIds(display_device::MacQueryType::Active)};
  ASSERT_FALSE(active_displays.empty());

  EXPECT_TRUE(std::ranges::any_of(active_displays, [this](const auto display_id) {
    return m_layer.isMainDisplay(display_id);
  }));
}

TEST_F_S(ChangeStubs) {
  EXPECT_FALSE(m_layer.setDisplayMode(1, {}));
  EXPECT_FALSE(m_layer.setOriginPoint(1, {}));
  EXPECT_FALSE(m_layer.setMirror(1, 2));
}
