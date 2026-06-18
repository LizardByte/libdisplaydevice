/**
 * @file src/macos/mac_api_layer.cpp
 * @brief Definitions for the MacApiLayer.
 */
// class header include
#include "display_device/macos/mac_api_layer.h"

// system includes
#include <sstream>

namespace display_device {
  bool MacApiLayer::isApiAccessAvailable() const {
    return true;
  }

  std::string MacApiLayer::getErrorString(const MacApiError error_code) const {
    std::ostringstream error;
    error << "[code: " << error_code << "]";
    return error.str();
  }

  MacDisplayIdList MacApiLayer::getDisplayIds(const MacQueryType type) const {
    static_cast<void>(type);
    return {};
  }

  std::optional<MacDisplayMode> MacApiLayer::getCurrentDisplayMode(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return std::nullopt;
  }

  MacDisplayModeList MacApiLayer::getDisplayModes(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return {};
  }

  std::string MacApiLayer::getDisplayName(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return {};
  }

  std::string MacApiLayer::getFriendlyName(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return {};
  }

  std::vector<std::byte> MacApiLayer::getEdid(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return {};
  }

  std::optional<Rational> MacApiLayer::getDisplayScale(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return std::nullopt;
  }

  std::optional<Point> MacApiLayer::getOriginPoint(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return std::nullopt;
  }

  bool MacApiLayer::isMainDisplay(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return false;
  }

  bool MacApiLayer::isActive(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return false;
  }

  bool MacApiLayer::isOnline(const MacDisplayId display_id) const {
    static_cast<void>(display_id);
    return false;
  }

  bool MacApiLayer::setDisplayMode(const MacDisplayId display_id, const MacDisplayMode &mode) {
    static_cast<void>(display_id);
    static_cast<void>(mode);
    return false;
  }

  bool MacApiLayer::setOriginPoint(const MacDisplayId display_id, const Point &origin) {
    static_cast<void>(display_id);
    static_cast<void>(origin);
    return false;
  }

  bool MacApiLayer::setMirror(const MacDisplayId display_id, const MacDisplayId master_display_id) {
    static_cast<void>(display_id);
    static_cast<void>(master_display_id);
    return false;
  }
}  // namespace display_device
