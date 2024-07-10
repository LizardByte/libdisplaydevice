#pragma once

// local includes
#include "audiocontextinterface.h"

namespace display_device {
  /**
   * @brief A no-operation implementation for AudioContextInterface.
   */
  class NoopAudioContext: public AudioContextInterface {
  public:
    /** Always returns true and sets m_is_captured to true. */
    [[nodiscard]] bool
    capture() override;

    /** Returns the m_is_captured value. */
    [[nodiscard]] bool
    isCaptured() const override;

    /** Sets m_is_captured to false. */
    void
    release() override;

  private:
    bool m_is_captured {};
  };
}  // namespace display_device
