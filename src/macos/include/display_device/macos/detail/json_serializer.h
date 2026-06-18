/**
 * @file src/macos/include/display_device/macos/detail/json_serializer.h
 * @brief Declarations for private JSON serialization helpers (macOS-only).
 */
#pragma once

// local includes
#include "display_device/detail/json_serializer.h"

#ifdef DD_JSON_DETAIL
namespace display_device {
  // Structs
  DD_JSON_DECLARE_SERIALIZE_TYPE(MacDisplayMode)
  DD_JSON_DECLARE_SERIALIZE_TYPE(MacSingleDisplayConfigState::Initial)
  DD_JSON_DECLARE_SERIALIZE_TYPE(MacSingleDisplayConfigState::Modified)
  DD_JSON_DECLARE_SERIALIZE_TYPE(MacSingleDisplayConfigState)
}  // namespace display_device
#endif
