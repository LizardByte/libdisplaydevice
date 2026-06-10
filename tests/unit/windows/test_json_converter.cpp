// local includes
#include "display_device/windows/json.h"
#include "fixtures/json_converter_test.h"
#include "utils/comparison.h"

// Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, JsonConverterTest, __VA_ARGS__)

TEST_F_S(ActiveTopology) {
  executeTestCase(display_device::ActiveTopology {}, R"([])");
  executeTestCase(display_device::ActiveTopology {{"DeviceId1"}, {"DeviceId2", "DeviceId3"}, {"DeviceId4"}}, R"([["DeviceId1"],["DeviceId2","DeviceId3"],["DeviceId4"]])");
  executeInvalidJsonTestCase<display_device::ActiveTopology>();
  executeFromJsonFailureTestCase<display_device::ActiveTopology>(R"([{}])");
  executeToJsonFailureTestCase(display_device::ActiveTopology {{"DeviceId1\xC2"}});
}

TEST_F_S(DeviceDisplayModeMap) {
  executeTestCase(display_device::DeviceDisplayModeMap {}, R"({})");
  executeTestCase(display_device::DeviceDisplayModeMap {{"DeviceId1", {}}, {"DeviceId2", {{1920, 1080}, {120, 1}}}}, R"({"DeviceId1":{"refresh_rate":{"denominator":0,"numerator":0},"resolution":{"height":0,"width":0}},"DeviceId2":{"refresh_rate":{"denominator":1,"numerator":120},"resolution":{"height":1080,"width":1920}}})");
  executeInvalidJsonTestCase<display_device::DeviceDisplayModeMap>();
  executeFromJsonFailureTestCase<display_device::DeviceDisplayModeMap>(R"({"DeviceId1":{}})");
  executeToJsonFailureTestCase(display_device::DeviceDisplayModeMap {{"DeviceId1\xC2", {}}});
}

TEST_F_S(HdrStateMap) {
  executeTestCase(display_device::HdrStateMap {}, R"({})");
  executeTestCase(display_device::HdrStateMap {{"DeviceId1", std::nullopt}, {"DeviceId2", display_device::HdrState::Enabled}}, R"({"DeviceId1":null,"DeviceId2":"Enabled"})");
  executeInvalidJsonTestCase<display_device::HdrStateMap>();
  executeFromJsonFailureTestCase<display_device::HdrStateMap>(R"({"DeviceId1":"OtherValue"})");
  executeToJsonFailureTestCase(display_device::HdrStateMap {{"DeviceId1\xC2", std::nullopt}});
  executeToJsonFailureTestCase(display_device::HdrStateMap {{"DeviceId1", static_cast<display_device::HdrState>(-1)}});
}

TEST_F_S(SingleDisplayConfigState) {
  const display_device::SingleDisplayConfigState valid_input {
    {{{"DeviceId1"}},
     {"DeviceId1"}},
    {display_device::SingleDisplayConfigState::Modified {
      {{"DeviceId2"}},
      {{"DeviceId2", {{1920, 1080}, {120, 1}}}},
      {{"DeviceId2", {display_device::HdrState::Disabled}}},
      {"DeviceId2"},
    }}
  };

  executeTestCase(display_device::SingleDisplayConfigState {}, R"({"initial":{"primary_devices":[],"topology":[]},"modified":{"original_hdr_states":{},"original_modes":{},"original_primary_device":"","topology":[]}})");
  executeTestCase(valid_input, R"({"initial":{"primary_devices":["DeviceId1"],"topology":[["DeviceId1"]]},"modified":{"original_hdr_states":{"DeviceId2":"Disabled"},"original_modes":{"DeviceId2":{"refresh_rate":{"denominator":1,"numerator":120},"resolution":{"height":1080,"width":1920}}},"original_primary_device":"DeviceId2","topology":[["DeviceId2"]]}})");
  executeInvalidJsonTestCase<display_device::SingleDisplayConfigState>();
  executeFromJsonFailureTestCase<display_device::SingleDisplayConfigState>(R"({})");
  executeToJsonFailureTestCase(display_device::SingleDisplayConfigState {
    .m_modified = {
      .m_original_primary_device = "DeviceId1\xC2",
    },
  });
}

TEST_F_S(WinWorkarounds) {
  display_device::WinWorkarounds input {
    std::chrono::milliseconds {500}
  };

  executeTestCase(display_device::WinWorkarounds {}, R"({"hdr_blank_delay":null})");
  executeTestCase(input, R"({"hdr_blank_delay":500})");
  executeInvalidJsonTestCase<display_device::WinWorkarounds>();
  executeFromJsonFailureTestCase<display_device::WinWorkarounds>(R"({})");
}
