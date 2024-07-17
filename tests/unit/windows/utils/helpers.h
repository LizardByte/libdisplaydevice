#pragma once

// system includes
#include <cstdint>

// local includes
#include "displaydevice/windows/types.h"
#include "displaydevice/windows/win_api_layer.h"

// Generic helper functions
std::optional<std::vector<std::string>>
getAvailableDevices(display_device::WinApiLayer &layer, bool only_valid_output = true);

std::optional<std::vector<std::uint8_t>>
serializeState(const std::optional<display_device::SingleDisplayConfigState> &state);
