#pragma once

// local includes
#include "displaydevice/windows/types.h"

// Helper comparison operators
bool
operator==(const LUID &lhs, const LUID &rhs);

bool
operator==(const POINTL &lhs, const POINTL &rhs);

bool
operator==(const RECTL &lhs, const RECTL &rhs);

bool
operator==(const DISPLAYCONFIG_RATIONAL &lhs, const DISPLAYCONFIG_RATIONAL &rhs);

bool
operator==(const DISPLAYCONFIG_2DREGION &lhs, const DISPLAYCONFIG_2DREGION &rhs);

bool
operator==(const DISPLAYCONFIG_PATH_SOURCE_INFO &lhs, const DISPLAYCONFIG_PATH_SOURCE_INFO &rhs);

bool
operator==(const DISPLAYCONFIG_PATH_TARGET_INFO &lhs, const DISPLAYCONFIG_PATH_TARGET_INFO &rhs);

bool
operator==(const DISPLAYCONFIG_PATH_INFO &lhs, const DISPLAYCONFIG_PATH_INFO &rhs);

bool
operator==(const DISPLAYCONFIG_SOURCE_MODE &lhs, const DISPLAYCONFIG_SOURCE_MODE &rhs);

bool
operator==(const DISPLAYCONFIG_VIDEO_SIGNAL_INFO &lhs, const DISPLAYCONFIG_VIDEO_SIGNAL_INFO &rhs);

bool
operator==(const DISPLAYCONFIG_TARGET_MODE &lhs, const DISPLAYCONFIG_TARGET_MODE &rhs);

bool
operator==(const DISPLAYCONFIG_DESKTOP_IMAGE_INFO &lhs, const DISPLAYCONFIG_DESKTOP_IMAGE_INFO &rhs);

bool
operator==(const DISPLAYCONFIG_MODE_INFO &lhs, const DISPLAYCONFIG_MODE_INFO &rhs);

bool
fuzzyCompare(float lhs, float rhs);

namespace display_device {
  bool
  operator==(const PathSourceIndexData &lhs, const PathSourceIndexData &rhs);

  bool
  operator==(const Point &lhs, const Point &rhs);

  bool
  operator==(const Rational &lhs, const Rational &rhs);

  bool
  operator==(const Resolution &lhs, const Resolution &rhs);

  bool
  operator==(const DisplayMode &lhs, const DisplayMode &rhs);

  bool
  operator==(const EnumeratedDevice::Info &lhs, const EnumeratedDevice::Info &rhs);

  bool
  operator==(const EnumeratedDevice &lhs, const EnumeratedDevice &rhs);
}  // namespace display_device
