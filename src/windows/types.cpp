/**
 * @file src/windows/types.cpp
 * @brief Definitions for Windows specific types.
 */
// header include
#include "display_device/windows/types.h"

namespace display_device {
  bool SingleDisplayConfigState::Modified::hasModifications() const {
    return !m_original_modes.empty() || !m_original_hdr_states.empty() || !m_original_primary_device.empty();
  }
}  // namespace display_device
