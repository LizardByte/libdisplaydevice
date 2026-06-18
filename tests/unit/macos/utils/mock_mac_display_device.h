#pragma once

// system includes
#include <gmock/gmock.h>

// local includes
#include "display_device/macos/mac_display_device_interface.h"

namespace display_device {
  class MockMacDisplayDevice: public MacDisplayDeviceInterface {
  public:
    MOCK_METHOD(bool, isApiAccessAvailable, (), (const, override));
    MOCK_METHOD(EnumeratedDeviceList, enumAvailableDevices, (), (const, override));
    MOCK_METHOD(std::string, getDisplayName, (const std::string &), (const, override));
    MOCK_METHOD(MacActiveTopology, getCurrentTopology, (), (const, override));
    MOCK_METHOD(bool, isTopologyValid, (const MacActiveTopology &), (const, override));
    MOCK_METHOD(bool, isTopologyTheSame, (const MacActiveTopology &, const MacActiveTopology &), (const, override));
    MOCK_METHOD(bool, setTopology, (const MacActiveTopology &), (override));
    MOCK_METHOD(MacDeviceDisplayModeMap, getCurrentDisplayModes, (const StringSet &), (const, override));
    MOCK_METHOD(bool, setDisplayModes, (const MacDeviceDisplayModeMap &), (override));
    MOCK_METHOD(bool, isPrimary, (const std::string &), (const, override));
    MOCK_METHOD(bool, setAsPrimary, (const std::string &), (override));
    MOCK_METHOD(MacHdrStateMap, getCurrentHdrStates, (const StringSet &), (const, override));
    MOCK_METHOD(bool, setHdrStates, (const MacHdrStateMap &), (override));
  };
}  // namespace display_device
