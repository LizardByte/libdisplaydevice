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
}  // namespace display_device
