// local includes
#include "fixtures/comparison.h"

bool
fuzzyCompare(float lhs, float rhs) {
  return std::abs(lhs - rhs) * 100000.f <= std::min(std::abs(lhs), std::abs(rhs));
}

namespace display_device {
  bool
  operator==(const Point &lhs, const Point &rhs) {
    return lhs.m_x == rhs.m_x && lhs.m_y == rhs.m_y;
  }

  bool
  operator==(const Resolution &lhs, const Resolution &rhs) {
    return lhs.m_height == rhs.m_height && lhs.m_width == rhs.m_width;
  }

  bool
  operator==(const EnumeratedDevice::Info &lhs, const EnumeratedDevice::Info &rhs) {
    return lhs.m_resolution == rhs.m_resolution && fuzzyCompare(lhs.m_resolution_scale, rhs.m_resolution_scale) &&
           fuzzyCompare(lhs.m_refresh_rate, rhs.m_refresh_rate) && lhs.m_primary == rhs.m_primary &&
           lhs.m_origin_point == rhs.m_origin_point && lhs.m_hdr_state == rhs.m_hdr_state;
  }

  bool
  operator==(const EnumeratedDevice &lhs, const EnumeratedDevice &rhs) {
    return lhs.m_device_id == rhs.m_device_id && lhs.m_display_name == rhs.m_display_name && lhs.m_friendly_name == rhs.m_friendly_name && lhs.m_info == rhs.m_info;
  }

  bool
  operator==(const SingleDisplayConfiguration &lhs, const SingleDisplayConfiguration &rhs) {
    return lhs.m_device_id == rhs.m_device_id && lhs.m_device_prep == rhs.m_device_prep && lhs.m_resolution == rhs.m_resolution && lhs.m_refresh_rate == rhs.m_refresh_rate && lhs.m_hdr_state == rhs.m_hdr_state;
  }
}  // namespace display_device
