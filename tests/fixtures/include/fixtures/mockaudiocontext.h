#pragma once

// system includes
#include <gmock/gmock.h>

// local includes
#include "displaydevice/audiocontextinterface.h"

namespace display_device {
  class MockAudioContext: public AudioContextInterface {
  public:
    MOCK_METHOD(bool, capture, (const std::vector<std::string> &), (override));
    MOCK_METHOD(void, release, (const std::vector<std::string> &), (override));
  };
}  // namespace display_device
