// class header include
#include "displaydevice/windows/windisplaydevice.h"

// system includes
#include <algorithm>
#include <unordered_set>

// local includes
#include "displaydevice/logging.h"
#include "displaydevice/windows/winutils.h"

namespace display_device {
  ActiveTopology
  WinDisplayDevice::getCurrentTopology() const {
    const auto display_data { m_w_api->queryDisplayConfig(QueryType::Active) };
    if (!display_data) {
      // Error already logged
      return {};
    }

    // Duplicate displays can be identified by having the same x/y position. Here we have a
    // "position to index" map for a simple and lazy lookup in case we have to add a device to the
    // topology group.
    std::unordered_map<std::string, std::size_t> position_to_topology_index;
    ActiveTopology topology;
    for (const auto &path : display_data->m_paths) {
      const auto device_info { win_utils::getDeviceInfoForValidPath(*m_w_api, path, true) };
      if (!device_info) {
        continue;
      }

      const auto source_mode { win_utils::getSourceMode(win_utils::getSourceIndex(path, display_data->m_modes), display_data->m_modes) };
      if (!source_mode) {
        DD_LOG(error) << "Active device does not have a source mode: " << device_info->m_device_id << "!";
        return {};
      }

      const std::string lazy_lookup { std::to_string(source_mode->position.x) + std::to_string(source_mode->position.y) };
      auto index_it { position_to_topology_index.find(lazy_lookup) };

      if (index_it == std::end(position_to_topology_index)) {
        position_to_topology_index[lazy_lookup] = topology.size();
        topology.push_back({ device_info->m_device_id });
      }
      else {
        topology.at(index_it->second).push_back(device_info->m_device_id);
      }
    }

    return topology;
  }

  bool
  WinDisplayDevice::isTopologyValid(const ActiveTopology &topology) const {
    if (topology.empty()) {
      DD_LOG(warning) << "Topology input is empty!";
      return false;
    }

    std::unordered_set<std::string> device_ids;
    for (const auto &group : topology) {
      // Size 2 is a Windows' limitation.
      // You CAN set the group to be more than 2, but then
      // Windows' settings app breaks since it was not designed for this :/
      if (group.empty() || group.size() > 2) {
        DD_LOG(warning) << "Topology group is invalid!";
        return false;
      }

      for (const auto &device_id : group) {
        if (!device_ids.insert(device_id).second) {
          DD_LOG(warning) << "Duplicate device ids found in topology!";
          return false;
        }
      }
    }

    return true;
  }

  bool
  WinDisplayDevice::isTopologyTheSame(const ActiveTopology &lhs, const ActiveTopology &rhs) const {
    const auto sort_topology = [](ActiveTopology &topology) {
      for (auto &group : topology) {
        std::sort(std::begin(group), std::end(group));
      }

      std::sort(std::begin(topology), std::end(topology));
    };

    auto lhs_copy { lhs };
    auto rhs_copy { rhs };

    // On Windows order does not matter.
    sort_topology(lhs_copy);
    sort_topology(rhs_copy);

    return lhs_copy == rhs_copy;
  }
}  // namespace display_device
