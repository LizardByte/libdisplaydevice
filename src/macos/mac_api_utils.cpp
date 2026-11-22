/**
 * @file src/macos/mac_api_utils.cpp
 * @brief Definitions for lower level macOS API utility functions.
 */
// header include
#include "display_device/macos/mac_api_utils.h"

// system includes
#include <algorithm>
#include <cmath>
#include <sstream>

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

  bool isUsableCgSerial(const std::uint32_t serial) {
    return serial != 0 && serial != 0xFFFFFFFF;
  }

  std::string makeCgDeviceIdentity(const CgDisplayInfo &display, const std::vector<CgDisplayInfo> &online_displays) {
    const auto shares_serial {[&display](const CgDisplayInfo &other) {
      return other.m_display_id != display.m_display_id && other.m_vendor == display.m_vendor && other.m_model == display.m_model && other.m_serial == display.m_serial;
    }};

    std::ostringstream identity;
    identity << "vendor=" << display.m_vendor << ';'
             << "model=" << display.m_model << ';'
             << "serial=" << display.m_serial;
    if (!isUsableCgSerial(display.m_serial) || std::ranges::any_of(online_displays, shares_serial)) {
      identity << ';' << "unit=" << display.m_unit << ';' << "display=" << display.m_display_id;
    }
    return identity.str();
  }
}  // namespace display_device::mac_utils
