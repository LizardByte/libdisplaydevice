// local includes
#include "display_device/macos/mac_api_layer.h"
#include "fixtures/fixtures.h"

namespace {
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
  EXPECT_EQ(m_layer.getErrorString(7), "[code: 7]");
}

TEST_F_S(QueryStubs) {
  EXPECT_TRUE(m_layer.getDisplayIds(display_device::MacQueryType::Active).empty());
  EXPECT_TRUE(m_layer.getDisplayIds(display_device::MacQueryType::Online).empty());
  EXPECT_EQ(m_layer.getCurrentDisplayMode(1), std::nullopt);
  EXPECT_TRUE(m_layer.getDisplayModes(1).empty());
  EXPECT_TRUE(m_layer.getDisplayName(1).empty());
  EXPECT_TRUE(m_layer.getFriendlyName(1).empty());
  EXPECT_TRUE(m_layer.getEdid(1).empty());
  EXPECT_EQ(m_layer.getDisplayScale(1), std::nullopt);
  EXPECT_EQ(m_layer.getOriginPoint(1), std::nullopt);
  EXPECT_FALSE(m_layer.isMainDisplay(1));
  EXPECT_FALSE(m_layer.isActive(1));
  EXPECT_FALSE(m_layer.isOnline(1));
}

TEST_F_S(ChangeStubs) {
  EXPECT_FALSE(m_layer.setDisplayMode(1, {}));
  EXPECT_FALSE(m_layer.setOriginPoint(1, {}));
  EXPECT_FALSE(m_layer.setMirror(1, 2));
}
