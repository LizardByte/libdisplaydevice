/**
 * @file src/macos/json_serializer.cpp
 * @brief Definitions for private JSON serialization helpers (macOS-only).
 */
// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "display_device/macos/types.h"
#include "display_device/macos/detail/json_serializer.h"
// clang-format on

namespace display_device {
  // Structs
  DD_JSON_DEFINE_SERIALIZE_STRUCT(MacDisplayMode, resolution, refresh_rate)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(MacSingleDisplayConfigState::Initial, topology, primary_devices)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(MacSingleDisplayConfigState::Modified, topology, original_modes, original_hdr_states, original_primary_device)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(MacSingleDisplayConfigState, initial, modified)
}  // namespace display_device
