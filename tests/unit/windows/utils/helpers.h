#pragma once

// system includes
#include <cstdint>
#include <format>
#include <gmock/gmock.h>
#include <string_view>

// local includes
#include "display_device/windows/types.h"
#include "display_device/windows/win_api_layer.h"

// Generic helper functions
std::optional<std::vector<std::string>> getAvailableDevices(const display_device::WinApiLayer &layer, bool only_valid_output = true);

std::optional<std::vector<std::uint8_t>> serializeState(const std::optional<display_device::SingleDisplayConfigState> &state);

template<class Layer>
decltype(auto) unwrapMockLayer(Layer &layer) {
  if constexpr (requires { *layer; }) {
    return *layer;
  } else {
    return (layer);
  }
}

template<class Layer>
void expectPathMetadataLookups(Layer &layer, const int count) {
  auto &mock_layer {unwrapMockLayer(layer)};
  for (int i = 1; i <= count; ++i) {
    EXPECT_CALL(mock_layer, getMonitorDevicePath(::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(std::format("Path{}", i)))
      .RetiresOnSaturation();
    EXPECT_CALL(mock_layer, getDeviceId(::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(std::format("DeviceId{}", i)))
      .RetiresOnSaturation();
    EXPECT_CALL(mock_layer, getDisplayName(::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(std::format("DisplayName{}", i)))
      .RetiresOnSaturation();
  }
}

template<class Layer>
void expectActivePathLookup(Layer &layer, const int id_number) {
  auto &mock_layer {unwrapMockLayer(layer)};
  for (int i = 1; i <= id_number; ++i) {
    EXPECT_CALL(mock_layer, getMonitorDevicePath(::testing::_))
      .Times(1)
      .WillOnce(::testing::Return("PathX"))
      .RetiresOnSaturation();
    EXPECT_CALL(mock_layer, getDeviceId(::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(std::format("DeviceId{}", i)))
      .RetiresOnSaturation();
    EXPECT_CALL(mock_layer, getDisplayName(::testing::_))
      .Times(1)
      .WillOnce(::testing::Return("DisplayNameX"))
      .RetiresOnSaturation();
  }
}

template<class Layer>
void expectDeviceIdLookups(Layer &layer, const int count) {
  auto &mock_layer {unwrapMockLayer(layer)};
  for (int i = 1; i <= count; ++i) {
    EXPECT_CALL(mock_layer, getDeviceId(::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(std::format("DeviceId{}", i)))
      .RetiresOnSaturation();
  }
}
