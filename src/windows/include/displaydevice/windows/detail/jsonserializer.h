#pragma once

// local includes
#include "displaydevice/detail/jsonserializer.h"

#ifdef DD_JSON_DETAIL
namespace display_device {
  // Structs
  DD_JSON_DECLARE_SERIALIZE_TYPE(DisplayMode)
  DD_JSON_DECLARE_SERIALIZE_TYPE(SingleDisplayConfigState::Initial)
  DD_JSON_DECLARE_SERIALIZE_TYPE(SingleDisplayConfigState::Modified)
  DD_JSON_DECLARE_SERIALIZE_TYPE(SingleDisplayConfigState)
}  // namespace display_device
#endif
