/**
 * @file src/windows/include/display_device/windows/detail/display_config.h
 * @brief Declarations for private DisplayConfig compatibility helpers.
 */
#pragma once

// local includes
#include "display_device/windows/types.h"

// system includes
#include <cstddef>
#include <cstring>

namespace display_device::detail {
  /**
   * @brief Gets desktop image metadata from a DisplayConfig mode record.
   * @param info Mode record whose union payload contains desktop image metadata.
   * @returns The desktop image metadata stored in the mode record.
   */
  [[nodiscard]] inline DISPLAYCONFIG_DESKTOP_IMAGE_INFO getDesktopImageInfo(const DISPLAYCONFIG_MODE_INFO &info) {
    constexpr std::size_t union_offset {offsetof(DISPLAYCONFIG_MODE_INFO, targetMode)};
    static_assert(sizeof(DISPLAYCONFIG_MODE_INFO) >= union_offset + sizeof(DISPLAYCONFIG_DESKTOP_IMAGE_INFO));

    DISPLAYCONFIG_DESKTOP_IMAGE_INFO desktop_image_info {};
    std::memcpy(&desktop_image_info, reinterpret_cast<const std::byte *>(&info) + union_offset, sizeof(desktop_image_info));

    return desktop_image_info;
  }

  /**
   * @brief Sets desktop image metadata on a DisplayConfig mode record.
   * @param info Mode record whose union payload should be updated.
   * @param desktop_image_info Desktop image metadata to store in the mode record.
   */
  inline void setDesktopImageInfo(DISPLAYCONFIG_MODE_INFO &info, const DISPLAYCONFIG_DESKTOP_IMAGE_INFO &desktop_image_info) {
    constexpr std::size_t union_offset {offsetof(DISPLAYCONFIG_MODE_INFO, targetMode)};
    static_assert(sizeof(DISPLAYCONFIG_MODE_INFO) >= union_offset + sizeof(DISPLAYCONFIG_DESKTOP_IMAGE_INFO));

    std::memcpy(reinterpret_cast<std::byte *>(&info) + union_offset, &desktop_image_info, sizeof(desktop_image_info));
  }
}  // namespace display_device::detail
