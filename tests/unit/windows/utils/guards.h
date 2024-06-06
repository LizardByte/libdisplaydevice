#pragma once

// system includes
#include <boost/scope/scope_exit.hpp>

// local includes
#include "displaydevice/windows/windisplaydevice.h"
#include "helpers.h"

// Helper functions to make guards for restoring previous state
inline auto
makeTopologyGuard(display_device::WinDisplayDevice &win_dd) {
  return boost::scope::make_scope_exit([&win_dd, topology = win_dd.getCurrentTopology()]() {
    static_cast<void>(win_dd.setTopology(topology));
  });
}

inline auto
makeModeGuard(display_device::WinDisplayDevice &win_dd) {
  return boost::scope::make_scope_exit([&win_dd, modes = win_dd.getCurrentDisplayModes(flattenTopology(win_dd.getCurrentTopology()))]() {
    static_cast<void>(win_dd.setDisplayModes(modes));
  });
}

inline auto
makePrimaryGuard(display_device::WinDisplayDevice &win_dd) {
  return boost::scope::make_scope_exit([&win_dd, primary_device = [&win_dd]() -> std::string {
    const auto flat_topology { flattenTopology(win_dd.getCurrentTopology()) };
    for (const auto &device_id : flat_topology) {
      if (win_dd.isPrimary(device_id)) {
        return device_id;
      }
    }

    return {};
  }()]() {
    static_cast<void>(win_dd.setAsPrimary(primary_device));
  });
}

inline auto
makeHdrStateGuard(display_device::WinDisplayDevice &win_dd) {
  return boost::scope::make_scope_exit([&win_dd, states = win_dd.getCurrentHdrStates(flattenTopology(win_dd.getCurrentTopology()))]() {
    static_cast<void>(win_dd.setHdrStates(states));
  });
}
