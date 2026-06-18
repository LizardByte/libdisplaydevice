// local includes
#include "display_device/macos/settings_utils.h"
#include "fixtures/fixtures.h"

// Specialized TEST macro(s) for this test file
#define TEST_S(...) DD_MAKE_TEST(TEST, MacSettingsUtils, __VA_ARGS__)

TEST_S(FlattenTopology) {
  EXPECT_EQ(display_device::mac_utils::flattenTopology({}), display_device::StringSet {});
  EXPECT_EQ(
    display_device::mac_utils::flattenTopology({{"DeviceId1"}, {"DeviceId2", "DeviceId3"}, {"DeviceId1"}}),
    (display_device::StringSet {"DeviceId1", "DeviceId2", "DeviceId3"})
  );
}

TEST_S(NoopGuard) {
  EXPECT_NO_THROW(display_device::mac_utils::noopGuard());
}
