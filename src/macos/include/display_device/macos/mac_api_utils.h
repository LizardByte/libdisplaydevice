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
}  // namespace display_device::mac_utils
