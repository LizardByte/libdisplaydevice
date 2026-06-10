#pragma once

// system includes
#include <gmock/gmock.h>

// local includes
#include "display_device/windows/win_display_device_interface.h"

namespace display_device {
  class MockWinDisplayDeviceAccess: public virtual WinDisplayDeviceInterface {
  public:
    MOCK_METHOD(bool, isApiAccessAvailable, (), (const, override));
    MOCK_METHOD(EnumeratedDeviceList, enumAvailableDevices, (), (const, override));
    MOCK_METHOD(std::string, getDisplayName, (const std::string &), (const, override));
  };

  class MockWinDisplayDeviceTopology: public virtual WinDisplayDeviceInterface {
  public:
    MOCK_METHOD(ActiveTopology, getCurrentTopology, (), (const, override));
    MOCK_METHOD(bool, isTopologyValid, (const ActiveTopology &), (const, override));
    MOCK_METHOD(bool, isTopologyTheSame, (const ActiveTopology &, const ActiveTopology &), (const, override));
    MOCK_METHOD(bool, setTopology, (const ActiveTopology &), (override));
  };

  class MockWinDisplayDeviceModes: public virtual WinDisplayDeviceInterface {
  public:
    MOCK_METHOD(DeviceDisplayModeMap, getCurrentDisplayModes, (const StringSet &), (const, override));
    MOCK_METHOD(bool, setDisplayModes, (const DeviceDisplayModeMap &), (override));
  };

  class MockWinDisplayDevicePrimary: public virtual WinDisplayDeviceInterface {
  public:
    MOCK_METHOD(bool, isPrimary, (const std::string &), (const, override));
    MOCK_METHOD(bool, setAsPrimary, (const std::string &), (override));
  };

  class MockWinDisplayDeviceHdr: public virtual WinDisplayDeviceInterface {
  public:
    MOCK_METHOD(HdrStateMap, getCurrentHdrStates, (const StringSet &), (const, override));
    MOCK_METHOD(bool, setHdrStates, (const HdrStateMap &), (override));
  };

  class MockWinDisplayDevice:
      public MockWinDisplayDeviceAccess,
      public MockWinDisplayDeviceTopology,
      public MockWinDisplayDeviceModes,
      public MockWinDisplayDevicePrimary,
      public MockWinDisplayDeviceHdr {};
}  // namespace display_device

/**
 * @brief Contains some useful predefined structures for UTs.
 * @note Data is to be extended with relevant information as needed.
 */
namespace ut_consts {
  extern const std::optional<display_device::SingleDisplayConfigState> SDCS_NULL;
  extern const std::optional<display_device::SingleDisplayConfigState> SDCS_EMPTY;
  extern const std::optional<display_device::SingleDisplayConfigState> SDCS_FULL;
  extern const std::optional<display_device::SingleDisplayConfigState> SDCS_NO_MODIFICATIONS;
}  // namespace ut_consts
