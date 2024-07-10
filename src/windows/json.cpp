// header include
#include "displaydevice/windows/json.h"

// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "displaydevice/windows/detail/jsonserializer.h"
#include "displaydevice/detail/jsonconverter.h"
// clang-format on

namespace display_device {
  const std::optional<unsigned int> JSON_COMPACT { std::nullopt };

  DD_JSON_DEFINE_CONVERTER(ActiveTopology)
  DD_JSON_DEFINE_CONVERTER(DeviceDisplayModeMap)
  DD_JSON_DEFINE_CONVERTER(HdrStateMap)
  DD_JSON_DEFINE_CONVERTER(SingleDisplayConfigState)
}  // namespace display_device
