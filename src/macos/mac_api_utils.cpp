/**
 * @file src/macos/mac_api_utils.cpp
 * @brief Definitions for lower level macOS API utility functions.
 */
// header include
#include "display_device/macos/mac_api_utils.h"

namespace display_device::mac_utils {
  bool isSuccess(const MacApiError error_code) {
    return error_code == 0;
  }
}  // namespace display_device::mac_utils
