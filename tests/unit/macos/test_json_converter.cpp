// local includes
#include "display_device/macos/json.h"
#include "fixtures/json_converter_test.h"

// Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, JsonConverterTest, __VA_ARGS__)

TEST_F_S(MacActiveTopology) {
  executeTestCase(display_device::MacActiveTopology {}, R"([])");
  executeTestCase(display_device::MacActiveTopology {{"DeviceId1"}, {"DeviceId2", "DeviceId3"}, {"DeviceId4"}}, R"([["DeviceId1"],["DeviceId2","DeviceId3"],["DeviceId4"]])");
  executeInvalidJsonTestCase<display_device::MacActiveTopology>();
  executeFromJsonFailureTestCase<display_device::MacActiveTopology>(R"([{}])");
  executeToJsonFailureTestCase(display_device::MacActiveTopology {{"DeviceId1\xC2"}});
}

TEST_F_S(MacDeviceDisplayModeMap) {
  executeTestCase(display_device::MacDeviceDisplayModeMap {}, R"({})");
  executeTestCase(
    display_device::MacDeviceDisplayModeMap {{"DeviceId1", {}}, {"DeviceId2", {{1920, 1080}, {120, 1}}}},
    R"({"DeviceId1":{"refresh_rate":{"denominator":0,"numerator":0},"resolution":{"height":0,"width":0}},"DeviceId2":{"refresh_rate":{"denominator":1,"numerator":120},"resolution":{"height":1080,"width":1920}}})"
  );
  executeInvalidJsonTestCase<display_device::MacDeviceDisplayModeMap>();
  executeFromJsonFailureTestCase<display_device::MacDeviceDisplayModeMap>(R"({"DeviceId1":{}})");
  executeToJsonFailureTestCase(display_device::MacDeviceDisplayModeMap {{"DeviceId1\xC2", {}}});
}

TEST_F_S(MacHdrStateMap) {
  executeTestCase(display_device::MacHdrStateMap {}, R"({})");
  executeTestCase(display_device::MacHdrStateMap {{"DeviceId1", std::nullopt}, {"DeviceId2", display_device::HdrState::Enabled}}, R"({"DeviceId1":null,"DeviceId2":"Enabled"})");
  executeInvalidJsonTestCase<display_device::MacHdrStateMap>();
  executeFromJsonFailureTestCase<display_device::MacHdrStateMap>(R"({"DeviceId1":"OtherValue"})");
  executeToJsonFailureTestCase(display_device::MacHdrStateMap {{"DeviceId1\xC2", std::nullopt}});
  executeToJsonFailureTestCase(display_device::MacHdrStateMap {{"DeviceId1", static_cast<display_device::HdrState>(-1)}});
}

TEST_F_S(MacSingleDisplayConfigState) {
  const display_device::MacSingleDisplayConfigState valid_input {
    {{{"DeviceId1"}},
     {"DeviceId1"}},
    {display_device::MacSingleDisplayConfigState::Modified {
      {{"DeviceId2"}},
      {{"DeviceId2", {{1920, 1080}, {120, 1}}}},
      {{"DeviceId2", {display_device::HdrState::Disabled}}},
      {"DeviceId2"},
    }}
  };

  executeTestCase(display_device::MacSingleDisplayConfigState {}, R"({"initial":{"primary_devices":[],"topology":[]},"modified":{"original_hdr_states":{},"original_modes":{},"original_primary_device":"","topology":[]}})");
  executeTestCase(valid_input, R"({"initial":{"primary_devices":["DeviceId1"],"topology":[["DeviceId1"]]},"modified":{"original_hdr_states":{"DeviceId2":"Disabled"},"original_modes":{"DeviceId2":{"refresh_rate":{"denominator":1,"numerator":120},"resolution":{"height":1080,"width":1920}}},"original_primary_device":"DeviceId2","topology":[["DeviceId2"]]}})");
  executeInvalidJsonTestCase<display_device::MacSingleDisplayConfigState>();
  executeFromJsonFailureTestCase<display_device::MacSingleDisplayConfigState>(R"({})");
  executeToJsonFailureTestCase(display_device::MacSingleDisplayConfigState {
    .m_modified = {
      .m_original_primary_device = "DeviceId1\xC2",
    },
  });
}
