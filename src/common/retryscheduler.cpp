// header include
#include "displaydevice/retryscheduler.h"

namespace display_device {
  SchedulerStopToken::SchedulerStopToken(std::function<void()> cleanup):
      m_cleanup { std::move(cleanup) } {
  }

  SchedulerStopToken::~SchedulerStopToken() {
    if (m_stop_requested && m_cleanup) {
      m_cleanup();
    }
  }

  void
  SchedulerStopToken::requestStop() {
    m_stop_requested = true;
  }

  bool
  SchedulerStopToken::stopRequested() const {
    return m_stop_requested;
  }
}  // namespace display_device
