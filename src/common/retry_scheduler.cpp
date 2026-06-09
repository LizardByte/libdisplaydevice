/**
 * @file src/common/retry_scheduler.cpp
 * @brief Definitions for the RetryScheduler.
 */
// header include
#include "display_device/retry_scheduler.h"

// system includes
#include <exception>
#include <iostream>

namespace display_device {
  namespace {
    void reportSchedulerStopTokenException(const std::exception &error) noexcept {
      std::cerr << "Exception thrown in SchedulerStopToken cleanup. Ignoring. Error:\n"
                << error.what() << '\n';
    }
  }  // namespace

  SchedulerStopToken::SchedulerStopToken(std::function<void()> cleanup):
      m_cleanup {std::move(cleanup)} {
  }

  SchedulerStopToken::~SchedulerStopToken() noexcept {
    if (m_stop_requested && m_cleanup) {
      try {
        m_cleanup();
      } catch (const std::exception &error) {
        reportSchedulerStopTokenException(error);
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
