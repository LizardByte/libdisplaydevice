// class header include
#include "displaydevice/windows/windisplaydevice.h"

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

  EnumeratedDeviceList
  WinDisplayDevice::enumAvailableDevices() const {
    const auto display_data { m_w_api->queryDisplayConfig(QueryType::All) };
    if (!display_data) {
      // Error already logged
      return {};
    }

    EnumeratedDeviceList available_devices;
    const auto source_data { win_utils::collectSourceDataForMatchingPaths(*m_w_api, display_data->m_paths) };
    if (source_data.empty()) {
      // Error already logged
      return {};
    }

    for (const auto &[device_id, data] : source_data) {
      const auto source_id_index { data.m_active_source.value_or(0) };
      const auto &best_path { display_data->m_paths.at(data.m_source_id_to_path_index.at(source_id_index)) };
      const auto friendly_name { m_w_api->getFriendlyName(best_path) };

      if (win_utils::isActive(best_path)) {
        const auto source_mode { win_utils::getSourceMode(win_utils::getSourceIndex(best_path, display_data->m_modes), display_data->m_modes) };
        if (!source_mode) {
          // Error already logged
          return {};
        }

        const auto display_name { m_w_api->getDisplayName(best_path) };
        const Rational refresh_rate { best_path.targetInfo.refreshRate.Denominator > 0 ?
                                        Rational { best_path.targetInfo.refreshRate.Numerator, best_path.targetInfo.refreshRate.Denominator } :
                                        Rational { 0, 1 } };
        const EnumeratedDevice::Info info {
          { source_mode->width, source_mode->height },
          m_w_api->getDisplayScale(display_name, *source_mode).value_or(Rational { 0, 1 }),
          refresh_rate,
          win_utils::isPrimary(*source_mode),
          { static_cast<int>(source_mode->position.x), static_cast<int>(source_mode->position.y) },
          m_w_api->getHdrState(best_path)
        };

        available_devices.push_back(
          { device_id,
            display_name,
            friendly_name,
            info });
      }
      else {
        available_devices.push_back(
          { device_id,
            std::string {},  // Inactive devices can have multiple display names, so it's just meaningless use any
            friendly_name,
            std::nullopt });
      }
    }

    return available_devices;
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
