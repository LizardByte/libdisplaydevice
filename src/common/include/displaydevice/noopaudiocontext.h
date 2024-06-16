#pragma once

// local includes
#include "audiocontextinterface.h"

namespace display_device {
  /**
   * @brief A no-operation implementation for AudioContextInterface.
   */
  class NoopAudioContext: public AudioContextInterface {
  public:
    /** Always returns true. */
    [[nodiscard]] bool
    capture(const std::vector<std::string> &) override;

    /** Does nothing. */
    void
    release(const std::vector<std::string> &) override;
  };
}  // namespace display_device
