#pragma once

// local includes
#include "jsonserializerdetails.h"

#ifdef DD_JSON_DETAIL
namespace display_device {
  // Enums
  DD_JSON_DECLARE_SERIALIZE_TYPE(HdrState)
  DD_JSON_DECLARE_SERIALIZE_TYPE(SingleDisplayConfiguration::DevicePreparation)

  // Structs
  DD_JSON_DECLARE_SERIALIZE_TYPE(Resolution)
  DD_JSON_DECLARE_SERIALIZE_TYPE(Rational)
  DD_JSON_DECLARE_SERIALIZE_TYPE(Point)
  DD_JSON_DECLARE_SERIALIZE_TYPE(EnumeratedDevice::Info)
  DD_JSON_DECLARE_SERIALIZE_TYPE(EnumeratedDevice)
  DD_JSON_DECLARE_SERIALIZE_TYPE(SingleDisplayConfiguration)
}  // namespace display_device
#endif
