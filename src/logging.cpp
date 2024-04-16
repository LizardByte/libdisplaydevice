// class header include
#include "logging.h"

// system includes
#include <chrono>
#include <format>
#include <iostream>
#include <mutex>

namespace display_device {
  logger_t &
  logger_t::get() {
    static logger_t instance;
    return instance;
  }

  void
  logger_t::set_log_level(const log_level_e log_level) {
    m_enabled_log_level = log_level;
  }

  bool
  logger_t::is_log_level_enabled(const log_level_e log_level) const {
    const auto log_level_v { static_cast<std::underlying_type_t<log_level_e>>(log_level) };
    const auto enabled_log_level_v { static_cast<std::underlying_type_t<log_level_e>>(m_enabled_log_level) };
    return log_level_v >= enabled_log_level_v;
  }

  void
  logger_t::set_custom_callback(callback_t callback) {
    m_custom_callback = std::move(callback);
  }

  void
  logger_t::write(const log_level_e log_level, std::string value) {
    if (!is_log_level_enabled(log_level)) {
      return;
    }

    if (m_custom_callback) {
      m_custom_callback(log_level, std::move(value));
      return;
    }

    std::stringstream stream;
    {
      // Time
      const auto now { std::chrono::current_zone()->to_local(std::chrono::system_clock::now()) };
      const auto now_ms { std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) };
      const auto now_s { std::chrono::duration_cast<std::chrono::seconds>(now_ms) };

      // we need to ensure that the time formatter does not print decimal numbers for seconds as
      // it currently has inconsistent formatting logic between platforms...
      // Instead we will do it manually.
      const auto now_local_seconds { std::chrono::local_seconds(now_s) };
      const auto now_decimal_part { now_ms - now_s };

      stream << std::format("[{:%Y-%m-%d %H:%M:%S}.{:0>3%Q}] ", now_local_seconds, now_decimal_part);

      // Log level
      switch (log_level) {
        case log_level_e::verbose:
          stream << "VERBOSE: ";
          break;
        case log_level_e::debug:
          stream << "DEBUG:   ";
          break;
        case log_level_e::info:
          stream << "INFO:    ";
          break;
        case log_level_e::warning:
          stream << "WARNING: ";
          break;
        case log_level_e::error:
          stream << "ERROR:   ";
          break;
        default:
          break;
      }

      // Value
      stream << value;
    }

    static std::mutex log_mutex;
    std::lock_guard lock { log_mutex };
    std::cout << stream.rdbuf() << std::endl;
  }

  logger_t::logger_t():
      m_enabled_log_level { log_level_e::info } {
  }

  log_writer_t::log_writer_t(const logger_t::log_level_e log_level):
      m_log_level { log_level } {}

  log_writer_t::~log_writer_t() {
    logger_t::get().write(m_log_level, m_buffer.str());
  }
}  // namespace display_device
