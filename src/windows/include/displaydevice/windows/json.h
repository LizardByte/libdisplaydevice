#pragma once

// local includes
#include "displaydevice/json.h"
#include "types.h"

// Windows' converters (add as needed)
namespace display_device {
  DD_JSON_DECLARE_CONVERTER(ActiveTopology)
  DD_JSON_DECLARE_CONVERTER(DeviceDisplayModeMap)
  DD_JSON_DECLARE_CONVERTER(HdrStateMap)
}  // namespace display_device
