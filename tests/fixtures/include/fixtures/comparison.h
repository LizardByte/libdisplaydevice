#pragma once

// local includes
#include "displaydevice/types.h"

// Helper comparison operators
bool
fuzzyCompare(float lhs, float rhs);

namespace display_device {
  bool
  operator==(const Point &lhs, const Point &rhs);

  bool
  operator==(const Resolution &lhs, const Resolution &rhs);

  bool
  operator==(const EnumeratedDevice::Info &lhs, const EnumeratedDevice::Info &rhs);

  bool
  operator==(const EnumeratedDevice &lhs, const EnumeratedDevice &rhs);
}  // namespace display_device
