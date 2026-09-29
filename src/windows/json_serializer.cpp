/**
 * @file src/windows/json_serializer.cpp
 * @brief Definitions for private JSON serialization helpers (Windows-only).
 */
// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "display_device/windows/types.h"
#include "display_device/windows/detail/json_serializer.h"
// clang-format on

namespace display_device {
  // Structs
  DD_JSON_DEFINE_SERIALIZE_STRUCT(DisplayMode, resolution, refresh_rate)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(SingleDisplayConfigState::Initial, topology, primary_devices)

  /**
   * @brief Serialize the modified state, including the interim primary device when there is one.
   * @param nlohmann_json_j JSON output object.
   * @param nlohmann_json_t Modified state to serialize.
   */
  void to_json(nlohmann::json &nlohmann_json_j, const SingleDisplayConfigState::Modified &nlohmann_json_t) {
    DD_JSON_TO(topology)
    DD_JSON_TO(original_modes)
    DD_JSON_TO(original_hdr_states)
    DD_JSON_TO(original_primary_device)
    if (!nlohmann_json_t.m_interim_primary_device.empty()) {
      DD_JSON_TO(interim_primary_device)
    }
  }

  /**
   * @brief Deserialize the modified state, treating a missing interim primary device as none.
   * @param nlohmann_json_j JSON input object.
   * @param nlohmann_json_t Modified state to populate.
   */
  void from_json(const nlohmann::json &nlohmann_json_j, SingleDisplayConfigState::Modified &nlohmann_json_t) {
    DD_JSON_FROM(topology)
    DD_JSON_FROM(original_modes)
    DD_JSON_FROM(original_hdr_states)
    DD_JSON_FROM(original_primary_device)
    nlohmann_json_t.m_interim_primary_device = nlohmann_json_j.value("interim_primary_device", std::string {});
  }

  DD_JSON_DEFINE_SERIALIZE_STRUCT(SingleDisplayConfigState, initial, modified)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(WinWorkarounds, hdr_blank_delay)
}  // namespace display_device
