/**
 * @file src/common/factory.cpp
 * @brief Factory helpers for unsupported platforms.
 */
// local includes
#include "display_device/factory.h"

#if !defined(_WIN32) && !defined(__APPLE__)

namespace display_device {
  std::unique_ptr<SettingsManagerInterface> makeSettingsManager(const SettingsManagerFactoryConfig &) {
    return nullptr;
  }

  std::unique_ptr<DisplayPowerInterface> makeDisplayPower() {
    return nullptr;
  }
}  // namespace display_device

#endif
