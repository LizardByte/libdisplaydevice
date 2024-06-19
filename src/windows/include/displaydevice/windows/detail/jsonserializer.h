#pragma once

// local includes
#include "displaydevice/detail/jsonserializer.h"

#ifdef DD_JSON_DETAIL
namespace display_device {
  // Structs
  DD_JSON_DECLARE_SERIALIZE_TYPE(Rational)
  DD_JSON_DECLARE_SERIALIZE_TYPE(DisplayMode)
}  // namespace display_device
#endif
