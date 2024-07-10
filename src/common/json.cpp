// header include
#include "displaydevice/json.h"

// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "displaydevice/detail/jsonserializer.h"
#include "displaydevice/detail/jsonconverter.h"
// clang-format on

namespace display_device {
  DD_JSON_DEFINE_CONVERTER(EnumeratedDeviceList)
  DD_JSON_DEFINE_CONVERTER(SingleDisplayConfiguration)
  DD_JSON_DEFINE_CONVERTER(std::set<std::string>)
  DD_JSON_DEFINE_CONVERTER(std::string)
  DD_JSON_DEFINE_CONVERTER(bool)
}  // namespace display_device
