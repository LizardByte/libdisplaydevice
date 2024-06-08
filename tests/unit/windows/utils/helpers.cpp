// local includes
#include "helpers.h"

std::set<std::string>
flattenTopology(const display_device::ActiveTopology &topology) {
  std::set<std::string> flattened_topology;
  for (const auto &group : topology) {
    for (const auto &device_id : group) {
      flattened_topology.insert(device_id);
    }
  }
  return flattened_topology;
}

std::optional<std::vector<std::string>>
getAvailableDevices(display_device::WinApiLayer &layer, const bool only_valid_output) {
  const auto all_devices { layer.queryDisplayConfig(display_device::QueryType::All) };
  if (!all_devices) {
    return std::nullopt;
  }

  std::set<std::string> device_ids;
  for (const auto &path : all_devices->m_paths) {
    if (only_valid_output && path.targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER) {
      continue;
    }

    const auto device_id { layer.getDeviceId(path) };
    if (!device_id.empty()) {
      device_ids.insert(device_id);
    }
  }

  return std::vector<std::string> { device_ids.begin(), device_ids.end() };
}
