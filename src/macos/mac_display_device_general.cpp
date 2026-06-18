/**
 * @file src/macos/mac_display_device_general.cpp
 * @brief Definitions for the leftover general methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

// system includes
#include <stdexcept>

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
    return {};
  }

  std::string MacDisplayDevice::getDisplayName(const std::string &device_id) const {
    static_cast<void>(device_id);
    return {};
  }
}  // namespace display_device
