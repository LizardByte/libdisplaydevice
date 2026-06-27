/**
 * @file src/macos/mac_display_device_general.cpp
 * @brief Definitions for the leftover general methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

// system includes
#include <stdexcept>

// local includes
#include "display_device/logging.h"

namespace display_device {
  MacDisplayDevice::MacDisplayDevice(std::shared_ptr<MacApiLayerInterface> m_api):
      m_m_api {std::move(m_api)} {
    if (!m_m_api) {
      throw std::invalid_argument {"Nullptr provided for MacApiLayerInterface in MacDisplayDevice!"};
    }
  }

  bool MacDisplayDevice::isApiAccessAvailable() const {
    return m_m_api->isApiAccessAvailable();
  }

  EnumeratedDeviceList MacDisplayDevice::enumAvailableDevices() const {
    EnumeratedDeviceList devices;
    StringSet seen_device_ids;

    for (const auto display_id : m_m_api->getDisplayIds(MacQueryType::Online)) {
      const auto device_id {m_m_api->getDeviceId(display_id)};
      if (device_id.empty() || !seen_device_ids.insert(device_id).second) {
        continue;
      }

      auto display_name {m_m_api->getDisplayName(display_id)};
      auto friendly_name {m_m_api->getFriendlyName(display_id)};
      if (friendly_name.empty()) {
        friendly_name = display_name;
      }

      const auto edid {EdidData::parse(m_m_api->getEdid(display_id))};

      std::optional<EnumeratedDevice::Info> info;
      if (m_m_api->isActive(display_id)) {
        if (const auto current_mode {m_m_api->getCurrentDisplayMode(display_id)}) {
          info = EnumeratedDevice::Info {
            current_mode->m_resolution,
            m_m_api->getDisplayScale(display_id).value_or(Rational {0, 1}),
            current_mode->m_refresh_rate,
            m_m_api->isMainDisplay(display_id),
            m_m_api->getOriginPoint(display_id).value_or(Point {}),
            std::nullopt
          };
        } else {
          DD_LOG(warning) << "Active macOS display is missing current mode: " << display_id;
        }
      }

      devices.push_back(EnumeratedDevice {device_id, display_name, friendly_name, edid, info});
    }

    return devices;
  }

  std::string MacDisplayDevice::getDisplayName(const std::string &device_id) const {
    const auto display_id {getDisplayId(device_id, MacQueryType::Online)};
    if (!display_id.has_value()) {
      return {};
    }

    return m_m_api->getDisplayName(*display_id);
  }

  std::optional<MacDisplayId> MacDisplayDevice::getDisplayId(const std::string_view device_id, const MacQueryType query_type) const {
    if (device_id.empty()) {
      return std::nullopt;
    }

    for (const auto display_id : m_m_api->getDisplayIds(query_type)) {
      if (m_m_api->getDeviceId(display_id) == device_id) {
        return display_id;
      }
    }

    return std::nullopt;
  }
}  // namespace display_device
