/**
 * @file src/macos/mac_display_device_modes.cpp
 * @brief Definitions for display mode related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

// system includes
#include <ranges>

// local includes
#include "display_device/logging.h"
#include "display_device/macos/mac_api_utils.h"

namespace display_device {
  namespace {
    /**
     * @brief Check if a mode list contains a matching mode.
     * @param modes Mode list to search.
     * @param mode Mode to search for.
     * @return True if the mode is available, false otherwise.
     */
    [[nodiscard]] bool hasMatchingMode(const MacDisplayModeList &modes, const MacDisplayMode &mode) {
      return std::ranges::any_of(modes, [&mode](const auto &candidate) {
        return mac_utils::fuzzyCompareModes(candidate, mode);
      });
    }
  }  // namespace

  MacDeviceDisplayModeMap MacDisplayDevice::getCurrentDisplayModes(const StringSet &device_ids) const {
    if (device_ids.empty()) {
      DD_LOG(error) << "Device id set is empty!";
      return {};
    }

    MacDeviceDisplayModeMap current_modes;
    for (const auto &device_id : device_ids) {
      const auto display_id {getDisplayId(device_id, MacQueryType::Active)};
      if (!display_id.has_value()) {
        DD_LOG(error) << "Failed to find active macOS display for " << device_id << "!";
        return {};
      }

      const auto current_mode {m_m_api->getCurrentDisplayMode(*display_id)};
      if (!current_mode) {
        DD_LOG(error) << "Failed to get current macOS display mode for " << device_id << "!";
        return {};
      }

      current_modes[device_id] = *current_mode;
    }

    return current_modes;
  }

  bool MacDisplayDevice::setDisplayModes(const MacDeviceDisplayModeMap &modes) {
    if (modes.empty()) {
      DD_LOG(error) << "Modes map is empty!";
      return false;
    }

    StringMap<MacDisplayId> display_ids;
    MacDeviceDisplayModeMap original_modes;
    for (const auto &[device_id, mode] : modes) {
      if (device_id.empty()) {
        DD_LOG(error) << "Device id is empty!";
        return false;
      }

      const auto display_id {getDisplayId(device_id, MacQueryType::Active)};
      if (!display_id.has_value()) {
        DD_LOG(error) << "Failed to find active macOS display for " << device_id << "!";
        return false;
      }

      const auto current_mode {m_m_api->getCurrentDisplayMode(*display_id)};
      if (!current_mode) {
        DD_LOG(error) << "Failed to get current macOS display mode for " << device_id << "!";
        return false;
      }

      if (!mac_utils::fuzzyCompareModes(*current_mode, mode) && !hasMatchingMode(m_m_api->getDisplayModes(*display_id), mode)) {
        DD_LOG(error) << "Requested macOS display mode is not available for " << device_id << "!";
        return false;
      }

      display_ids[device_id] = *display_id;
      original_modes[device_id] = *current_mode;
    }

    MacDeviceDisplayModeMap changed_modes;
    for (const auto &[device_id, mode] : modes) {
      if (mac_utils::fuzzyCompareModes(original_modes.at(device_id), mode)) {
        continue;
      }

      const auto display_id {display_ids.at(device_id)};
      if (!m_m_api->setDisplayMode(display_id, mode)) {
        DD_LOG(error) << "Failed to set macOS display mode for " << device_id << "!";
        if (!changed_modes.empty()) {
          static_cast<void>(setDisplayModes(changed_modes));
        }
        return false;
      }

      const auto verified_mode {m_m_api->getCurrentDisplayMode(display_id)};
      if (!verified_mode || !mac_utils::fuzzyCompareModes(*verified_mode, mode)) {
        DD_LOG(error) << "Failed to verify macOS display mode for " << device_id << "!";
        changed_modes[device_id] = original_modes.at(device_id);
        if (!changed_modes.empty()) {
          static_cast<void>(setDisplayModes(changed_modes));
        }
        return false;
      }

      changed_modes[device_id] = original_modes.at(device_id);
    }

    if (changed_modes.empty()) {
      DD_LOG(debug) << "No changes were made to macOS display modes as they are equal.";
    }

    return true;
  }
}  // namespace display_device
