/**
 * @file src/macos/include/display_device/macos/mac_api_utils.h
 * @brief Declarations for lower level macOS API utility functions.
 */
#pragma once

// system includes
#include <cstdint>
#include <string>

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

  /**
   * @brief Build the identity text hashed into a "macos-cg-*" device id.
   *
   * Used when a display has neither an EDID nor IOKit metadata, which on Apple Silicon is most
   * displays (there are no IODisplayConnect services) and every virtual display. The display id
   * and unit number are reassigned when a display reconnects or a virtual display is recreated, so
   * they are only included when the display reports no serial number; vendor, model and a nonzero
   * serial already identify it.
   * @param vendor CGDisplayVendorNumber value.
   * @param model CGDisplayModelNumber value.
   * @param serial CGDisplaySerialNumber value.
   * @param unit CGDisplayUnitNumber value.
   * @param display_id Display id.
   * @returns Text to hash.
   */
  [[nodiscard]] std::string makeCgDeviceIdentity(std::uint32_t vendor, std::uint32_t model, std::uint32_t serial, std::uint32_t unit, MacDisplayId display_id);
}  // namespace display_device::mac_utils
