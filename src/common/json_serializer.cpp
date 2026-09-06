/**
 * @file src/common/json_serializer.cpp
 * @brief Definitions for private JSON serialization helpers.
 */
// special ordered include of details
#define DD_JSON_DETAIL
// clang-format off
#include "display_device/types.h"
#include "display_device/detail/json_serializer.h"
// clang-format on

namespace display_device {
  // Enums
  DD_JSON_DEFINE_SERIALIZE_ENUM_GCOVR_EXCL_BR_LINE(HdrState, {{HdrState::Disabled, "Disabled"}, {HdrState::Enabled, "Enabled"}})
  DD_JSON_DEFINE_SERIALIZE_ENUM_GCOVR_EXCL_BR_LINE(SingleDisplayConfiguration::DevicePreparation, {{SingleDisplayConfiguration::DevicePreparation::VerifyOnly, "VerifyOnly"}, {SingleDisplayConfiguration::DevicePreparation::EnsureActive, "EnsureActive"}, {SingleDisplayConfiguration::DevicePreparation::EnsurePrimary, "EnsurePrimary"}, {SingleDisplayConfiguration::DevicePreparation::EnsureOnlyDisplay, "EnsureOnlyDisplay"}})

  // Structs
  DD_JSON_DEFINE_SERIALIZE_STRUCT(Resolution, width, height)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(Rational, numerator, denominator)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(Point, x, y)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(EdidData, manufacturer_id, product_code, serial_number)
  DD_JSON_DEFINE_SERIALIZE_STRUCT(EnumeratedDevice::Info, resolution, resolution_scale, refresh_rate, primary, origin_point, hdr_state)

  /**
   * @brief Serialize an enumerated display, including its internal-panel classification.
   * @param nlohmann_json_j JSON output object.
   * @param nlohmann_json_t Device to serialize.
   */
  void to_json(nlohmann::json &nlohmann_json_j, const EnumeratedDevice &nlohmann_json_t) {
    DD_JSON_TO(device_id)
    DD_JSON_TO(display_name)
    DD_JSON_TO(friendly_name)
    DD_JSON_TO(edid)
    DD_JSON_TO(info)
    DD_JSON_TO(is_internal)
  }

  /**
   * @brief Deserialize a display, treating missing internal-panel information as unknown.
   * @param nlohmann_json_j JSON input object.
   * @param nlohmann_json_t Device to populate.
   */
  void from_json(const nlohmann::json &nlohmann_json_j, EnumeratedDevice &nlohmann_json_t) {
    DD_JSON_FROM(device_id)
    DD_JSON_FROM(display_name)
    DD_JSON_FROM(friendly_name)
    DD_JSON_FROM(edid)
    DD_JSON_FROM(info)
    nlohmann_json_t.m_is_internal = nlohmann_json_j.value("is_internal", false);
  }
  DD_JSON_DEFINE_SERIALIZE_STRUCT(SingleDisplayConfiguration, device_id, device_prep, resolution, refresh_rate, hdr_state)
}  // namespace display_device
