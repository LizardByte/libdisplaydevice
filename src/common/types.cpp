// header include
#include "displaydevice/types.h"

namespace {
  bool
  fuzzyCompare(const double lhs, const double rhs) {
    return std::abs(lhs - rhs) * 1000000000000. <= std::min(std::abs(lhs), std::abs(rhs));
  }

  bool
  fuzzyCompare(const display_device::FloatingPoint &lhs, const display_device::FloatingPoint &rhs) {
    if (lhs.index() == rhs.index()) {
      if (std::holds_alternative<double>(lhs)) {
        return fuzzyCompare(std::get<double>(lhs), std::get<double>(rhs));
      }
      return lhs == rhs;
    }
    return false;
  }
}  // namespace

namespace display_device {
  bool
  operator==(const Rational &lhs, const Rational &rhs) {
    return lhs.m_numerator == rhs.m_numerator && lhs.m_denominator == rhs.m_denominator;
  }

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
