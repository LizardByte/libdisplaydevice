// local includes
#include "display_device/macos/mac_api_utils.h"
#include "fixtures/fixtures.h"

// Specialized TEST macro(s) for this test file
#define TEST_S(...) DD_MAKE_TEST(TEST, MacApiUtils, __VA_ARGS__)

TEST_S(IsSuccess) {
  EXPECT_TRUE(display_device::mac_utils::isSuccess(0));
  EXPECT_FALSE(display_device::mac_utils::isSuccess(1));
  EXPECT_FALSE(display_device::mac_utils::isSuccess(-1));
}

TEST_S(FuzzyCompareRefreshRates) {
  EXPECT_TRUE(display_device::mac_utils::fuzzyCompareRefreshRates({60, 1}, {5985, 100}));
  EXPECT_TRUE(display_device::mac_utils::fuzzyCompareRefreshRates({60, 1}, {5920, 100}));
  EXPECT_FALSE(display_device::mac_utils::fuzzyCompareRefreshRates({60, 1}, {5900, 100}));
  EXPECT_FALSE(display_device::mac_utils::fuzzyCompareRefreshRates({60, 0}, {5985, 100}));
  EXPECT_FALSE(display_device::mac_utils::fuzzyCompareRefreshRates({60, 1}, {5985, 0}));
}

TEST_S(FuzzyCompareModes) {
  EXPECT_TRUE(display_device::mac_utils::fuzzyCompareModes({{1920, 1080}, {60, 1}}, {{1920, 1080}, {5985, 100}}));
  EXPECT_FALSE(display_device::mac_utils::fuzzyCompareModes({{1280, 1080}, {60, 1}}, {{1920, 1080}, {60, 1}}));
  EXPECT_FALSE(display_device::mac_utils::fuzzyCompareModes({{1920, 720}, {60, 1}}, {{1920, 1080}, {60, 1}}));
  EXPECT_FALSE(display_device::mac_utils::fuzzyCompareModes({{1920, 1080}, {60, 1}}, {{1920, 1080}, {50, 1}}));
}

TEST_S(MakeCgDeviceIdentity) {
  using display_device::mac_utils::makeCgDeviceIdentity;

  // With a serial, the identity survives the display id and unit number changing (reconnects,
  // virtual displays being recreated).
  EXPECT_EQ(makeCgDeviceIdentity(22084, 1, 1398099505, 2, 9), "vendor=22084;model=1;serial=1398099505");
  EXPECT_EQ(makeCgDeviceIdentity(22084, 1, 1398099505, 2, 9), makeCgDeviceIdentity(22084, 1, 1398099505, 3, 30));
  EXPECT_NE(makeCgDeviceIdentity(22084, 1, 1398099505, 2, 9), makeCgDeviceIdentity(22084, 1, 1398099506, 2, 9));

  // Without one, the display id and unit number are all that tell identical displays apart.
  EXPECT_EQ(makeCgDeviceIdentity(1552, 41007, 0, 0, 1), "vendor=1552;model=41007;serial=0;unit=0;display=1");
  EXPECT_NE(makeCgDeviceIdentity(1552, 41007, 0, 0, 1), makeCgDeviceIdentity(1552, 41007, 0, 1, 2));
}
