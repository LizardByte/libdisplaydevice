// local includes
#include "displaydevice/windows/json.h"
#include "fixtures/jsonconvertertest.h"
#include "utils/comparison.h"

namespace {
  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, JsonConverterTest, __VA_ARGS__)
}  // namespace

TEST_F_S(ActiveTopology) {
  executeTestCase(display_device::ActiveTopology {}, R"([])");
  executeTestCase(display_device::ActiveTopology { { "DeviceId1" }, { "DeviceId2", "DeviceId3" }, { "DeviceId4" } }, R"([["DeviceId1"],["DeviceId2","DeviceId3"],["DeviceId4"]])");
}

TEST_F_S(DeviceDisplayModeMap) {
  executeTestCase(display_device::DeviceDisplayModeMap {}, R"({})");
  executeTestCase(display_device::DeviceDisplayModeMap { { "DeviceId1", {} }, { "DeviceId2", { { 1920, 1080 }, { 120, 1 } } } },
    R"({"DeviceId1":{"refresh_rate":{"denominator":0,"numerator":0},"resolution":{"height":0,"width":0}},"DeviceId2":{"refresh_rate":{"denominator":1,"numerator":120},"resolution":{"height":1080,"width":1920}}})");
}

TEST_F_S(HdrStateMap) {
  executeTestCase(display_device::HdrStateMap {}, R"({})");
  executeTestCase(display_device::HdrStateMap { { "DeviceId1", std::nullopt }, { "DeviceId2", display_device::HdrState::Enabled } },
    R"({"DeviceId1":null,"DeviceId2":"Enabled"})");
}
