// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "displaydevice/windows/types.h"
#include "displaydevice/windows/detail/jsonserializer.h"
// clang-format on

namespace display_device {
  // Structs
  DD_JSON_DEFINE_SERIALIZE_STRUCT(Rational, numerator, denominator)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(DisplayMode, resolution, refresh_rate)
}  // namespace display_device
