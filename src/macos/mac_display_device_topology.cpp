/**
 * @file src/macos/mac_display_device_topology.cpp
 * @brief Definitions for topology related methods in MacDisplayDevice.
 */
// class header include
#include "display_device/macos/mac_display_device.h"

// system includes
#include <algorithm>

namespace display_device {
  MacActiveTopology MacDisplayDevice::getCurrentTopology() const {
    std::vector<std::pair<MacDisplayId, std::vector<std::string>>> groups;

    for (const auto display_id : m_m_api->getDisplayIds(MacQueryType::Active)) {
      const auto device_id {m_m_api->getDeviceId(display_id)};
      if (device_id.empty()) {
        continue;
      }

      const auto mirror_master {m_m_api->getMirrorMaster(display_id)};
      const auto group_key {mirror_master != 0 ? mirror_master : display_id};
      auto group_it {std::ranges::find_if(groups, [group_key](const auto &group) {
        return group.first == group_key;
      })};

      if (group_it == std::end(groups)) {
        groups.emplace_back(group_key, std::vector<std::string> {device_id});
      } else {
        group_it->second.push_back(device_id);
      }
    }

    MacActiveTopology topology;
    topology.reserve(groups.size());
    for (auto &group : groups) {
      topology.push_back(std::move(group.second));
    }

    return topology;
  }

  bool MacDisplayDevice::isTopologyValid(const MacActiveTopology &topology) const {
    if (topology.empty()) {
      return false;
    }

    StringUnorderedSet device_ids;
    for (const auto &group : topology) {
      if (group.empty()) {
        return false;
      }

      for (const auto &device_id : group) {
        if (device_id.empty() || !device_ids.insert(device_id).second) {
          return false;
        }
      }
    }

    return true;
  }

  bool MacDisplayDevice::isTopologyTheSame(const MacActiveTopology &lhs, const MacActiveTopology &rhs) const {
    const auto sort_topology = [](MacActiveTopology &topology) {
      for (auto &group : topology) {
        std::ranges::sort(group);
      }

      std::ranges::sort(topology);
    };

    auto lhs_copy {lhs};
    auto rhs_copy {rhs};
    sort_topology(lhs_copy);
    sort_topology(rhs_copy);
    return lhs_copy == rhs_copy;
  }

  bool MacDisplayDevice::setTopology(const MacActiveTopology &new_topology) {
    static_cast<void>(new_topology);
    return false;
  }
}  // namespace display_device
