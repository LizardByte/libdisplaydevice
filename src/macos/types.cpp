/**
 * @file src/macos/types.cpp
 * @brief Definitions for macOS specific types.
 */
// header include
#include "display_device/macos/types.h"

namespace display_device {
  bool MacSingleDisplayConfigState::Modified::hasModifications() const {
    return !m_original_modes.empty() || !m_original_hdr_states.empty() || !m_original_primary_device.empty();
  }
}  // namespace display_device
