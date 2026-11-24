/**
 * @file src/macos/include/display_device/macos/mac_api_utils.h
 * @brief Declarations for lower level macOS API utility functions.
 */
#pragma once

// system includes
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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
   * @brief CoreGraphics values that identify a display without an EDID or IOKit metadata.
   */
  struct CgDisplayInfo {
    MacDisplayId m_display_id;  ///< Display id (reassigned when a display reconnects).
    std::uint32_t m_vendor;  ///< CGDisplayVendorNumber value.
    std::uint32_t m_model;  ///< CGDisplayModelNumber value.
    std::uint32_t m_serial;  ///< CGDisplaySerialNumber value.
    std::uint32_t m_unit;  ///< CGDisplayUnitNumber value (depends on connection order).
  };

  /**
   * @brief Check whether a CGDisplaySerialNumber value identifies a display.
   * @param serial CGDisplaySerialNumber value.
   * @returns False for 0 (no serial) and 0xFFFFFFFF (documented by Apple as an exception value).
   */
  [[nodiscard]] bool isUsableCgSerial(std::uint32_t serial);

  /**
   * @brief Build the identity text hashed into a "macos-cg-*" device id.
   *
   * Used when a display has neither an EDID nor IOKit metadata, which on Apple Silicon is most
   * displays (there are no IODisplayConnect services) and every virtual display. The display id
   * and unit number change when a display reconnects or a virtual display is recreated, so they are
   * left out when vendor, model and serial identify the display on their own: the serial is usable
   * and no other online display reports the same vendor, model and serial. Otherwise they are kept,
   * so displays that can't be told apart by serial still get distinct ids.
   * @param display Display to build the identity for.
   * @param online_displays Every online display (may include `display` itself).
   * @returns Text to hash.
   */
  [[nodiscard]] std::string makeCgDeviceIdentity(const CgDisplayInfo &display, const std::vector<CgDisplayInfo> &online_displays);

  /**
   * @brief Build the other identity text a display's "macos-cg-*" device id may have been saved with.
   *
   * Whether makeCgDeviceIdentity() includes the display id and unit number depends on whether
   * another online display reports the same vendor, model and serial, so a display that stays
   * connected while such a display is plugged in or unplugged switches between the two forms. This
   * returns the form makeCgDeviceIdentity() does not currently give, so a saved id can still be
   * matched. Displays without a usable serial only ever have one form.
   * @param display Display to build the identity for.
   * @param online_displays Every online display (may include `display` itself).
   * @returns The other identity text, or nothing if the display has only one.
   */
  [[nodiscard]] std::optional<std::string> makeCgDeviceIdentityAlias(const CgDisplayInfo &display, const std::vector<CgDisplayInfo> &online_displays);
}  // namespace display_device::mac_utils
