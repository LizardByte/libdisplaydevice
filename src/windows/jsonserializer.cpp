// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "displaydevice/windows/types.h"
#include "displaydevice/windows/detail/jsonserializer.h"
// clang-format on

namespace display_device {
  // Structs
  DD_JSON_DEFINE_SERIALIZE_STRUCT(DisplayMode, resolution, refresh_rate)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(SingleDisplayConfigState::Initial, topology, primary_devices)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(SingleDisplayConfigState::Modified, topology, original_modes, original_hdr_states, original_primary_device)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(SingleDisplayConfigState, initial, modified)
}  // namespace display_device
