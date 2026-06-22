/**
 * @file src/macos/mac_api_utils.cpp
 * @brief Definitions for lower level macOS API utility functions.
 */
// header include
#include "display_device/macos/mac_api_utils.h"

// system includes
#include <cmath>

namespace display_device::mac_utils {
  bool isSuccess(const MacApiError error_code) {
    return error_code == 0;
  }

  bool fuzzyCompareRefreshRates(const Rational &lhs, const Rational &rhs) {
    if (lhs.m_denominator > 0 && rhs.m_denominator > 0) {
      const double lhs_value {static_cast<double>(lhs.m_numerator) / static_cast<double>(lhs.m_denominator)};
      const double rhs_value {static_cast<double>(rhs.m_numerator) / static_cast<double>(rhs.m_denominator)};
      return std::abs(lhs_value - rhs_value) <= 0.9;
    }

    return false;
  }

  bool fuzzyCompareModes(const MacDisplayMode &lhs, const MacDisplayMode &rhs) {
    return lhs.m_resolution == rhs.m_resolution &&
           fuzzyCompareRefreshRates(lhs.m_refresh_rate, rhs.m_refresh_rate);
  }
}  // namespace display_device::mac_utils
