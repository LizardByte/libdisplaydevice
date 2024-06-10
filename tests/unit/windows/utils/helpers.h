#pragma once

// system includes
#include <set>

// local includes
#include "displaydevice/windows/types.h"
#include "displaydevice/windows/winapilayer.h"

// Generic helper functions
std::set<std::string>
flattenTopology(const display_device::ActiveTopology &topology);

std::optional<std::vector<std::string>>
getAvailableDevices(display_device::WinApiLayer &layer, bool only_valid_output = true);
