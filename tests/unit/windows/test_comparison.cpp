// local includes
#include "display_device/windows/detail/display_config.h"
#include "display_device/windows/types.h"
#include "fixtures/fixtures.h"
#include "utils/comparison.h"

// Specialized TEST macro(s) for this test file
#define TEST_S(...) DD_MAKE_TEST(TEST, TypeComparison, __VA_ARGS__)

TEST_S(DisplayMode) {
  EXPECT_EQ(display_device::DisplayMode({1, 1}, {1, 1}), display_device::DisplayMode({1, 1}, {1, 1}));
  EXPECT_NE(display_device::DisplayMode({1, 1}, {1, 1}), display_device::DisplayMode({1, 0}, {1, 1}));
  EXPECT_NE(display_device::DisplayMode({1, 1}, {1, 1}), display_device::DisplayMode({1, 1}, {1, 0}));
}

TEST_S(DisplayConfigDesktopImageMode) {
  auto make_mode {
    []() {
      DISPLAYCONFIG_MODE_INFO mode {};
      mode.infoType = DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE;
      mode.id = 1;
      mode.adapterId = {2, 3};
      const DISPLAYCONFIG_DESKTOP_IMAGE_INFO desktop_image_info {
        .PathSourceSize = {3840, 2160},
        .DesktopImageRegion = {0, 0, 3840, 2160},
        .DesktopImageClip = {10, 20, 3830, 2140},
      };
      display_device::detail::setDesktopImageInfo(mode, desktop_image_info);

      return mode;
    }
  };

  auto lhs {make_mode()};
  auto rhs {make_mode()};
  EXPECT_EQ(lhs, rhs);

  const DISPLAYCONFIG_DESKTOP_IMAGE_INFO changed_desktop_image_info {
    .PathSourceSize = {3840, 2160},
    .DesktopImageRegion = {0, 0, 3840, 2160},
    .DesktopImageClip = {10, 20, 1920, 2140},
  };
  display_device::detail::setDesktopImageInfo(rhs, changed_desktop_image_info);
  EXPECT_NE(lhs, rhs);
}

TEST_S(SingleDisplayConfigState, Initial) {
  using Initial = display_device::SingleDisplayConfigState::Initial;
  EXPECT_EQ(Initial({{{"1"}}}, {"1"}), Initial({{{"1"}}}, {"1"}));
  EXPECT_NE(Initial({{{"1"}}}, {"1"}), Initial({{{"0"}}}, {"1"}));
  EXPECT_NE(Initial({{{"1"}}}, {"1"}), Initial({{{"1"}}}, {"0"}));
}

TEST_S(SingleDisplayConfigState, Modified) {
  using Modified = display_device::SingleDisplayConfigState::Modified;
  EXPECT_EQ(Modified({{{"1"}}}, {{"1", {}}}, {{"1", {}}}, "1"), Modified({{{"1"}}}, {{"1", {}}}, {{"1", {}}}, "1"));
  EXPECT_NE(Modified({{{"1"}}}, {{"1", {}}}, {{"1", {}}}, "1"), Modified({{{"0"}}}, {{"1", {}}}, {{"1", {}}}, "1"));
  EXPECT_NE(Modified({{{"1"}}}, {{"1", {}}}, {{"1", {}}}, "1"), Modified({{{"1"}}}, {{"0", {}}}, {{"1", {}}}, "1"));
  EXPECT_NE(Modified({{{"1"}}}, {{"1", {}}}, {{"1", {}}}, "1"), Modified({{{"1"}}}, {{"1", {}}}, {{"0", {}}}, "1"));
  EXPECT_NE(Modified({{{"1"}}}, {{"1", {}}}, {{"1", {}}}, "1"), Modified({{{"1"}}}, {{"1", {}}}, {{"1", {}}}, "0"));
}

TEST_S(SingleDisplayConfigState) {
  using SDSC = display_device::SingleDisplayConfigState;
  EXPECT_EQ(SDSC({{{"1"}}}, {{{"1"}}}), SDSC({{{"1"}}}, {{{"1"}}}));
  EXPECT_NE(SDSC({{{"1"}}}, {{{"1"}}}), SDSC({{{"0"}}}, {{{"1"}}}));
  EXPECT_NE(SDSC({{{"1"}}}, {{{"1"}}}), SDSC({{{"1"}}}, {{{"0"}}}));
}
