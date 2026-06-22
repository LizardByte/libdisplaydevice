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
