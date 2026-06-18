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
