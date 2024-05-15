#pragma once

// system includes
#include <boost/scope/scope_exit.hpp>

// local includes
#include "displaydevice/windows/windisplaydevice.h"

// Helper functions to make guards for restoring previous state
auto
makeTopologyGuard(display_device::WinDisplayDevice &win_dd) {
  return boost::scope::make_scope_exit([&win_dd, topology = win_dd.getCurrentTopology()]() {
    static_cast<void>(win_dd.setTopology(topology));
  });
}