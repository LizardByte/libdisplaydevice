// system includes
#include <format>

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

TEST_S(IsUsableCgSerial) {
  EXPECT_TRUE(display_device::mac_utils::isUsableCgSerial(1398099505));
  EXPECT_FALSE(display_device::mac_utils::isUsableCgSerial(0));
  EXPECT_FALSE(display_device::mac_utils::isUsableCgSerial(0xFFFFFFFF));
}

TEST_S(MakeCgDeviceIdentity) {
  using display_device::mac_utils::CgDisplayInfo;
  using display_device::mac_utils::makeCgDeviceIdentity;

  // A unique serial: the identity survives the display id and unit number changing (reconnects,
  // virtual displays being recreated).
  const CgDisplayInfo virtual_display {9, 22084, 1, 1398099505, 2};
  const CgDisplayInfo recreated {30, 22084, 1, 1398099505, 3};
  const CgDisplayInfo other_panel {1, 1552, 41007, 7, 0};
  EXPECT_EQ(makeCgDeviceIdentity(virtual_display, {virtual_display, other_panel}), "vendor=22084;model=1;serial=1398099505");
  EXPECT_EQ(makeCgDeviceIdentity(virtual_display, {virtual_display}), makeCgDeviceIdentity(recreated, {recreated}));

  // No usable serial (0, or Apple's 0xFFFFFFFF exception value): the display id and unit number
  // are all that tell identical displays apart.
  for (const auto serial : {0U, 0xFFFFFFFFU}) {
    const CgDisplayInfo first {1, 1552, 41007, serial, 0};
    const CgDisplayInfo second {2, 1552, 41007, serial, 1};
    EXPECT_EQ(makeCgDeviceIdentity(first, {first}), std::format("vendor=1552;model=41007;serial={};unit=0;display=1", serial));
    EXPECT_NE(makeCgDeviceIdentity(first, {first, second}), makeCgDeviceIdentity(second, {first, second}));
  }

  // Two online displays reporting the same vendor, model and serial keep distinct identities.
  const CgDisplayInfo twin_a {4, 1552, 41007, 16843009, 0};
  const CgDisplayInfo twin_b {5, 1552, 41007, 16843009, 1};
  EXPECT_EQ(makeCgDeviceIdentity(twin_a, {twin_a, twin_b}), "vendor=1552;model=41007;serial=16843009;unit=0;display=4");
  EXPECT_NE(makeCgDeviceIdentity(twin_a, {twin_a, twin_b}), makeCgDeviceIdentity(twin_b, {twin_a, twin_b}));
  // Once the twin is gone, the remaining display gets the stable identity again.
  EXPECT_EQ(makeCgDeviceIdentity(twin_a, {twin_a}), "vendor=1552;model=41007;serial=16843009");
}

TEST_S(MakeCgDeviceIdentityAlias) {
  using display_device::mac_utils::CgDisplayInfo;
  using display_device::mac_utils::makeCgDeviceIdentity;
  using display_device::mac_utils::makeCgDeviceIdentityAlias;

  const CgDisplayInfo twin_a {4, 1552, 41007, 16843009, 0};
  const CgDisplayInfo twin_b {5, 1552, 41007, 16843009, 1};

  // A saved alone, then B plugged in: A's saved id becomes its alias, and B has the same alias.
  const auto saved_alone {makeCgDeviceIdentity(twin_a, {twin_a})};
  EXPECT_EQ(makeCgDeviceIdentityAlias(twin_a, {twin_a, twin_b}), saved_alone);
  EXPECT_EQ(makeCgDeviceIdentityAlias(twin_b, {twin_a, twin_b}), saved_alone);

  // A saved with B online, then B unplugged (A keeps its display id and unit): A's saved id becomes
  // its alias, which only A has.
  const auto saved_with_twin {makeCgDeviceIdentity(twin_a, {twin_a, twin_b})};
  EXPECT_EQ(makeCgDeviceIdentityAlias(twin_a, {twin_a}), saved_with_twin);
  EXPECT_NE(makeCgDeviceIdentityAlias(twin_b, {twin_b}), saved_with_twin);

  // No usable serial: only one form.
  for (const auto serial : {0U, 0xFFFFFFFFU}) {
    const CgDisplayInfo display {1, 1552, 41007, serial, 0};
    EXPECT_EQ(makeCgDeviceIdentityAlias(display, {display}), std::nullopt);
  }
}
