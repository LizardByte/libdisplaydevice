// header include
#include "displaydevice/windows/types.h"

// system includes
#include <cmath>

namespace display_device {
  Rational
  Rational::fromFloatingPoint(const double value) {
    // It's hard to deal with floating values, so we just multiply it
    // to keep 4 decimal places (if any) and let Windows deal with it!
    // Genius idea if I'm being honest.
    constexpr auto multiplier { static_cast<unsigned int>(std::pow(10, 4)) };
    const double transformed_value { std::round(value * multiplier) };
    return Rational { static_cast<unsigned int>(transformed_value), multiplier };
  }

  bool
  operator==(const Rational &lhs, const Rational &rhs) {
    return lhs.m_numerator == rhs.m_numerator && lhs.m_denominator == rhs.m_denominator;
  }

  bool
  operator==(const DisplayMode &lhs, const DisplayMode &rhs) {
    return lhs.m_refresh_rate == rhs.m_refresh_rate && lhs.m_resolution == rhs.m_resolution;
  }

  bool
  SingleDisplayConfigState::Modified::hasModifications() const {
    return !m_original_modes.empty() || !m_original_hdr_states.empty() || !m_original_primary_device.empty();
  }

  bool
  operator==(const SingleDisplayConfigState::Initial &lhs, const SingleDisplayConfigState::Initial &rhs) {
    return lhs.m_topology == rhs.m_topology && lhs.m_primary_devices == rhs.m_primary_devices;
  }

  bool
  operator==(const SingleDisplayConfigState::Modified &lhs, const SingleDisplayConfigState::Modified &rhs) {
    return lhs.m_topology == rhs.m_topology && lhs.m_original_modes == rhs.m_original_modes && lhs.m_original_hdr_states == rhs.m_original_hdr_states && lhs.m_original_primary_device == rhs.m_original_primary_device;
  }

  bool
  operator==(const SingleDisplayConfigState &lhs, const SingleDisplayConfigState &rhs) {
    return lhs.m_initial == rhs.m_initial && lhs.m_modified == rhs.m_modified;
  }
}  // namespace display_device
