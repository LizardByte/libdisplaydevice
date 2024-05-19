// class header include
#include "displaydevice/logging.h"

// system includes
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace display_device {
  Logger &
  Logger::get() {
    static Logger instance;
    return instance;
  }

  void
  Logger::setLogLevel(LogLevel log_level) {
    m_enabled_log_level = log_level;
  }

  bool
  Logger::isLogLevelEnabled(LogLevel log_level) const {
    const auto log_level_v { static_cast<std::underlying_type_t<LogLevel>>(log_level) };
    const auto enabled_log_level_v { static_cast<std::underlying_type_t<LogLevel>>(m_enabled_log_level) };
    return log_level_v >= enabled_log_level_v;
  }

  void
  Logger::setCustomCallback(Callback callback) {
    m_custom_callback = std::move(callback);
  }

  void
  Logger::write(const LogLevel log_level, std::string value) {
    if (!isLogLevelEnabled(log_level)) {
      return;
    }

    if (m_custom_callback) {
      m_custom_callback(log_level, std::move(value));
      return;
    }

    std::stringstream stream;
    {
      // Time
      {
        static const auto get_time { []() {
          static const auto to_year_month_day { [](const auto &time) {
            return std::chrono::year_month_day { std::chrono::time_point_cast<std::chrono::days>(time) };
          } };
          static const auto to_hour_minute_second { [](const auto &time) {
            const auto start_of_day { std::chrono::floor<std::chrono::days>(time) };
            const auto time_since_start_of_day { std::chrono::round<std::chrono::seconds>(time - start_of_day) };
            return std::chrono::hh_mm_ss { time_since_start_of_day };
          } };
          static const auto to_milliseconds { [](const auto &now, const auto &time) {
            const auto now_ms { std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) };
            const auto time_s { std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()) };
            return now_ms - time_s;
          } };

          const auto now { std::chrono::system_clock::now() };
          const auto time_zone { std::chrono::current_zone() };
          if (time_zone) {
            const auto local_time { time_zone->to_local(now) };
            return std::make_tuple(to_year_month_day(local_time), to_hour_minute_second(local_time), to_milliseconds(now, local_time));
          }
          return std::make_tuple(to_year_month_day(now), to_hour_minute_second(now), to_milliseconds(now, now));
        } };

        const auto [year_month_day, hh_mm_ss, ms] { get_time() };
        const auto old_flags { stream.flags() };  // Save formatting flags so that they can be restored...

        stream << "[" << year_month_day << " " << hh_mm_ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        stream.flags(old_flags);
      }

      // Log level
      switch (log_level) {
        case LogLevel::verbose:
          stream << "VERBOSE: ";
          break;
        case LogLevel::debug:
          stream << "DEBUG:   ";
          break;
        case LogLevel::info:
          stream << "INFO:    ";
          break;
        case LogLevel::warning:
          stream << "WARNING: ";
          break;
        case LogLevel::error:
          stream << "ERROR:   ";
          break;
      }

      // Value
      stream << value;
    }

    static std::mutex log_mutex;
    std::lock_guard lock { log_mutex };
    std::cout << stream.rdbuf() << std::endl;
  }

  Logger::Logger():
      m_enabled_log_level { LogLevel::info } {
  }

  LogWriter::LogWriter(const Logger::LogLevel log_level):
      m_log_level { log_level } {}

  LogWriter::~LogWriter() {
    Logger::get().write(m_log_level, m_buffer.str());
  }
}  // namespace display_device
