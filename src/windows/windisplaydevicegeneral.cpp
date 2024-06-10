// class header include
#include "displaydevice/windows/windisplaydevice.h"

// system includes
#include <stdexcept>

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/windows/winapiutils.h"

namespace display_device {
  WinDisplayDevice::WinDisplayDevice(std::shared_ptr<WinApiLayerInterface> w_api):
      m_w_api { std::move(w_api) } {
    if (!m_w_api) {
      throw std::logic_error { "Nullptr provided for WinApiLayerInterface in WinDisplayDevice!" };
    }
  }

  bool
  WinDisplayDevice::isApiAccessAvailable() const {
    const auto display_data { m_w_api->queryDisplayConfig(QueryType::All) };
    if (!display_data) {
      DD_LOG(debug) << "WinDisplayDevice::isApiAccessAvailable failed while querying display data.";
      return false;
    }

    // Here we are supplying the retrieved display data back to SetDisplayConfig (with VALIDATE flag only, so that we make no actual changes).
    // Unless something is really broken on Windows, this call should never fail under normal circumstances - the configuration is 100% correct, since it was
    // provided by Windows.
    const UINT32 flags { SDC_VALIDATE | SDC_USE_SUPPLIED_DISPLAY_CONFIG | SDC_VIRTUAL_MODE_AWARE };
    const LONG result { m_w_api->setDisplayConfig(display_data->m_paths, display_data->m_modes, flags) };

    DD_LOG(debug) << "WinDisplayDevice::isApiAccessAvailable result: " << m_w_api->getErrorString(result);
    return result == ERROR_SUCCESS;
  }

  std::string
  WinDisplayDevice::getDisplayName(const std::string &device_id) const {
    if (device_id.empty()) {
      // Valid return, no error
      return {};
    }

    const auto display_data { m_w_api->queryDisplayConfig(QueryType::Active) };
    if (!display_data) {
      // Error already logged
      return {};
    }

    const auto path { win_utils::getActivePath(*m_w_api, device_id, display_data->m_paths) };
    if (!path) {
      // Debug level, because inactive device is valid case for this function
      DD_LOG(debug) << "Failed to find device for " << device_id << "!";
      return {};
    }

    const auto display_name { m_w_api->getDisplayName(*path) };
    if (display_name.empty()) {
      // Theoretically possible due to some race condition in the OS...
      DD_LOG(error) << "Device " << device_id << " has no display name assigned.";
    }

    return display_name;
  }
}  // namespace display_device
