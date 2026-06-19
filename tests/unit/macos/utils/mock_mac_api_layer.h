#pragma once

// system includes
#include <gmock/gmock.h>

// local includes
#include "display_device/macos/mac_api_layer_interface.h"

namespace display_device {
  class MockMacApiLayer: public MacApiLayerInterface {
  public:
    MOCK_METHOD(bool, isApiAccessAvailable, (), (const, override));
    MOCK_METHOD(std::string, getErrorString, (MacApiError), (const, override));
    MOCK_METHOD(MacDisplayIdList, getDisplayIds, (MacQueryType), (const, override));
    MOCK_METHOD(std::string, getDeviceId, (MacDisplayId), (const, override));
    MOCK_METHOD(std::optional<MacDisplayMode>, getCurrentDisplayMode, (MacDisplayId), (const, override));
    MOCK_METHOD(MacDisplayModeList, getDisplayModes, (MacDisplayId), (const, override));
    MOCK_METHOD(std::string, getDisplayName, (MacDisplayId), (const, override));
    MOCK_METHOD(std::string, getFriendlyName, (MacDisplayId), (const, override));
    MOCK_METHOD(std::vector<std::byte>, getEdid, (MacDisplayId), (const, override));
    MOCK_METHOD(std::optional<Rational>, getDisplayScale, (MacDisplayId), (const, override));
    MOCK_METHOD(std::optional<Point>, getOriginPoint, (MacDisplayId), (const, override));
    MOCK_METHOD(bool, isMainDisplay, (MacDisplayId), (const, override));
    MOCK_METHOD(bool, isActive, (MacDisplayId), (const, override));
    MOCK_METHOD(bool, isOnline, (MacDisplayId), (const, override));
    MOCK_METHOD(MacDisplayId, getMirrorMaster, (MacDisplayId), (const, override));
    MOCK_METHOD(bool, setDisplayMode, (MacDisplayId, const MacDisplayMode &), (override));
    MOCK_METHOD(bool, setOriginPoint, (MacDisplayId, const Point &), (override));
    MOCK_METHOD(bool, setMirror, (MacDisplayId, MacDisplayId), (override));
  };
}  // namespace display_device
