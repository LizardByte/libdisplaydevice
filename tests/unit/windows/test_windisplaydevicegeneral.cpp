// system includes
#include <gmock/gmock.h>

// local includes
#include "displaydevice/windows/windisplaydevice.h"
#include "fixtures.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::HasSubstr;
  
  // Specialized TEST macro(s) for this test file
#define TEST_S(...) DD_MAKE_TEST(TEST, WinDisplayDeviceGeneral, __VA_ARGS__)
}  // namespace

TEST_S(NullptrLayerProvided) {
  EXPECT_THAT([]() { const auto win_dd { display_device::WinDisplayDevice { nullptr } }; },
    ThrowsMessage<std::logic_error>(HasSubstr("Nullptr provided for WinApiLayerInterface in WinDisplayDevice!")));
}
