/**
 * @file src/macos/json.cpp
 * @brief Definitions for JSON conversion functions (macOS-only).
 */
// header include
#include "display_device/macos/json.h"

// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "display_device/macos/detail/json_serializer.h"
#include "display_device/detail/json_converter.h"
// clang-format on

namespace display_device {
  DD_JSON_DEFINE_CONVERTER(MacActiveTopology)
  DD_JSON_DEFINE_CONVERTER(MacDeviceDisplayModeMap)
  DD_JSON_DEFINE_CONVERTER(MacHdrStateMap)
  DD_JSON_DEFINE_CONVERTER(MacSingleDisplayConfigState)
}  // namespace display_device
