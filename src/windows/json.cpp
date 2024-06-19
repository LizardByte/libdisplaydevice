// header include
#include "displaydevice/windows/json.h"

// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "displaydevice/windows/detail/jsonserializer.h"
#include "displaydevice/detail/jsonconverter.h"
// clang-format on

namespace display_device {
  DD_JSON_DEFINE_CONVERTER(ActiveTopology)
  DD_JSON_DEFINE_CONVERTER(DeviceDisplayModeMap)
  DD_JSON_DEFINE_CONVERTER(HdrStateMap)
}  // namespace display_device
