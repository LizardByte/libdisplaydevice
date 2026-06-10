/**
 * @file src/common/include/display_device/noop_audio_context.h
 * @brief Declarations for the NoopAudioContext.
 */
#pragma once

// local includes
#include "audio_context_interface.h"

namespace display_device {
  /**
   * @brief A no-operation implementation for AudioContextInterface.
   */
  class NoopAudioContext: public AudioContextInterface {
  public:
    /**
     * @copydoc AudioContextInterface::capture
     */
    [[nodiscard]] bool capture() override;

    /**
     * @copydoc AudioContextInterface::isCaptured
     */
    [[nodiscard]] bool isCaptured() const override;

    /**
     * @copydoc AudioContextInterface::release
     */
    void release() override;

  private:
    bool m_is_captured {};
  };
}  // namespace display_device
