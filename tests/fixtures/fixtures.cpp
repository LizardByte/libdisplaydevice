// header include
#include "fixtures/fixtures.h"

// lib includes
#include <lizardbyte/common/env.h>

void BaseTest::SetUp() {
  m_test_skipped_at_setup = false;
  ::lizardbyte::common::testing::BaseTest::SetUp();
  m_test_skipped_at_setup = ::testing::Test::IsSkipped();
  if (m_test_skipped_at_setup) {
    return;
  }

  // Set the default log level, before the test starts. Will default to verbose in case nothing was specified.
  display_device::Logger::get().setLogLevel(getDefaultLogLevel().value_or(display_device::Logger::LogLevel::verbose));
}

void BaseTest::TearDown() {
  if (!m_test_skipped_at_setup) {
    // reset the callback to avoid potential leaks
    display_device::Logger::get().setCustomCallback(nullptr);
  }

  ::lizardbyte::common::testing::BaseTest::TearDown();
}

std::optional<display_device::Logger::LogLevel> BaseTest::getDefaultLogLevel() const {
  const static auto default_log_level {
    []() -> std::optional<display_device::Logger::LogLevel> {
      using enum display_device::Logger::LogLevel;

      const auto value {lizardbyte::common::get_env("LOG_LEVEL")};
      if (value == "verbose") {
        return verbose;
      }
      if (value == "debug") {
        return debug;
      }
      if (value == "info") {
        return info;
      }
      if (value == "warning") {
        return warning;
      }
      if (value == "error") {
        return error;
      }

      return std::nullopt;
    }()
  };

  return default_log_level;
}
