/**
 * @file src/macos/include/display_device/macos/json.h
 * @brief Declarations for JSON conversion functions (macOS-only).
 */
#pragma once

// local includes
#include "display_device/json.h"
#include "types.h"

namespace display_device {
  DD_JSON_DECLARE_CONVERTER(MacActiveTopology)
  DD_JSON_DECLARE_CONVERTER(MacDeviceDisplayModeMap)
  DD_JSON_DECLARE_CONVERTER(MacHdrStateMap)
  DD_JSON_DECLARE_CONVERTER(MacSingleDisplayConfigState)
}  // namespace display_device
