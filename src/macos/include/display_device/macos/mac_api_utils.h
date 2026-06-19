/**
 * @file src/macos/include/display_device/macos/mac_api_utils.h
 * @brief Declarations for lower level macOS API utility functions.
 */
#pragma once

// local includes
#include "types.h"

/**
 * @brief Shared utility-level code for macOS API wrappers.
 */
namespace display_device::mac_utils {
  /**
   * @brief Check if a macOS API error represents success.
   * @param error_code Error code to check.
   * @returns True if the error code represents success, false otherwise.
   */
  [[nodiscard]] bool isSuccess(MacApiError error_code);

  /**
   * @brief Check if two refresh rates are close enough to be treated as equal.
   * @param lhs First refresh rate to compare.
   * @param rhs Second refresh rate to compare.
   * @returns True if the refresh rates are close enough, false otherwise.
   */
  [[nodiscard]] bool fuzzyCompareRefreshRates(const Rational &lhs, const Rational &rhs);

  /**
   * @brief Check if two macOS display modes are close enough to be treated as equal.
   * @param lhs First mode to compare.
   * @param rhs Second mode to compare.
   * @returns True if resolution matches exactly and refresh rate is close enough.
   */
  [[nodiscard]] bool fuzzyCompareModes(const MacDisplayMode &lhs, const MacDisplayMode &rhs);
}  // namespace display_device::mac_utils
