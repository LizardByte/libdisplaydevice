// local includes
#include "fixtures/jsonconvertertest.h"

namespace {
  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, JsonConverterTest, __VA_ARGS__)
}  // namespace

TEST_F_S(EnumeratedDeviceList) {
  display_device::EnumeratedDevice item_1 {
    "ID_1",
    "NAME_2",
    "FU_NAME_3",
    display_device::EnumeratedDevice::Info {
      { 1920, 1080 },
      1.75f,
      119.9554f,
      false,
      { 1, 2 },
      display_device::HdrState::Enabled }
  };
  display_device::EnumeratedDevice item_2 {
    "ID_2",
    "NAME_2",
    "FU_NAME_2",
    display_device::EnumeratedDevice::Info {
      { 1920, 1080 },
      1.75f,
      119.9554f,
      true,
      { 0, 0 },
      display_device::HdrState::Disabled }
  };
  display_device::EnumeratedDevice item_3 {};

  executeTestCase(display_device::EnumeratedDeviceList {}, R"([])");
  executeTestCase(display_device::EnumeratedDeviceList { item_1, item_2, item_3 },
    R"([{"device_id":"ID_1","display_name":"NAME_2","friendly_name":"FU_NAME_3","info":{"hdr_state":"Enabled","origin_point":{"x":1,"y":2},"primary":false,"refresh_rate":119.95539855957031,"resolution":{"height":1080,"width":1920},"resolution_scale":1.75}},)"
    R"({"device_id":"ID_2","display_name":"NAME_2","friendly_name":"FU_NAME_2","info":{"hdr_state":"Disabled","origin_point":{"x":0,"y":0},"primary":true,"refresh_rate":119.95539855957031,"resolution":{"height":1080,"width":1920},"resolution_scale":1.75}},)"
    R"({"device_id":"","display_name":"","friendly_name":"","info":null}])");
}

TEST_F_S(SingleDisplayConfiguration) {
  display_device::SingleDisplayConfiguration config_1 { "ID_1", display_device::SingleDisplayConfiguration::DevicePreparation::VerifyOnly, { { 156, 123 } }, 85.f, display_device::HdrState::Enabled };
  display_device::SingleDisplayConfiguration config_2 { "ID_2", display_device::SingleDisplayConfiguration::DevicePreparation::EnsureActive, std::nullopt, 85.f, display_device::HdrState::Disabled };
  display_device::SingleDisplayConfiguration config_3 { "ID_3", display_device::SingleDisplayConfiguration::DevicePreparation::EnsureOnlyDisplay, { { 156, 123 } }, std::nullopt, std::nullopt };
  display_device::SingleDisplayConfiguration config_4 { "ID_4", display_device::SingleDisplayConfiguration::DevicePreparation::EnsurePrimary, std::nullopt, std::nullopt, std::nullopt };

  executeTestCase(display_device::SingleDisplayConfiguration {}, R"({"device_id":"","device_prep":"VerifyOnly","hdr_state":null,"refresh_rate":null,"resolution":null})");
  executeTestCase(config_1, R"({"device_id":"ID_1","device_prep":"VerifyOnly","hdr_state":"Enabled","refresh_rate":85.0,"resolution":{"height":123,"width":156}})");
  executeTestCase(config_2, R"({"device_id":"ID_2","device_prep":"EnsureActive","hdr_state":"Disabled","refresh_rate":85.0,"resolution":null})");
  executeTestCase(config_3, R"({"device_id":"ID_3","device_prep":"EnsureOnlyDisplay","hdr_state":null,"refresh_rate":null,"resolution":{"height":123,"width":156}})");
  executeTestCase(config_4, R"({"device_id":"ID_4","device_prep":"EnsurePrimary","hdr_state":null,"refresh_rate":null,"resolution":null})");
}
