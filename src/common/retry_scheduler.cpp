/**
 * @file src/common/retry_scheduler.cpp
 * @brief Definitions for the RetryScheduler.
 */
// header include
#include "display_device/retry_scheduler.h"

namespace display_device {
  SchedulerStopToken::SchedulerStopToken(std::function<void()> cleanup):
      m_cleanup {std::move(cleanup)} {
  }

  SchedulerStopToken::~SchedulerStopToken() noexcept {
    if (m_stop_requested && m_cleanup) {
      try {
        m_cleanup();
      } catch (...) {
        // Destructors must not propagate cleanup failures.
      }
    }
  }

  void SchedulerStopToken::requestStop() {
    m_stop_requested = true;
  }

  bool SchedulerStopToken::stopRequested() const {
    return m_stop_requested;
  }
}  // namespace display_device
