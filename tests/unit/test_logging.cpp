// system includes
#include <regex>

// local includes
#include "tests/conftest.cpp"

TEST(LoggingTest, LogLevelVerbose) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  logger.set_log_level(level::verbose);

  EXPECT_EQ(logger.is_log_level_enabled(level::verbose), true);
  EXPECT_EQ(logger.is_log_level_enabled(level::debug), true);
  EXPECT_EQ(logger.is_log_level_enabled(level::info), true);
  EXPECT_EQ(logger.is_log_level_enabled(level::error), true);
}

TEST(LoggingTest, LogLevelDebug) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  logger.set_log_level(level::debug);

  EXPECT_EQ(logger.is_log_level_enabled(level::verbose), false);
  EXPECT_EQ(logger.is_log_level_enabled(level::debug), true);
  EXPECT_EQ(logger.is_log_level_enabled(level::info), true);
  EXPECT_EQ(logger.is_log_level_enabled(level::error), true);
}

TEST(LoggingTest, LogLevelInfo) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  logger.set_log_level(level::info);

  EXPECT_EQ(logger.is_log_level_enabled(level::verbose), false);
  EXPECT_EQ(logger.is_log_level_enabled(level::debug), false);
  EXPECT_EQ(logger.is_log_level_enabled(level::info), true);
  EXPECT_EQ(logger.is_log_level_enabled(level::error), true);
}

TEST(LoggingTest, LogLevelError) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  logger.set_log_level(level::error);

  EXPECT_EQ(logger.is_log_level_enabled(level::verbose), false);
  EXPECT_EQ(logger.is_log_level_enabled(level::debug), false);
  EXPECT_EQ(logger.is_log_level_enabled(level::info), false);
  EXPECT_EQ(logger.is_log_level_enabled(level::error), true);
}

TEST(LoggingTest, DefaultLogger) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  const auto write_and_get_cout { [this, &logger](level level, std::string value) -> std::string {
    this->cout_buffer.str(std::string {});  // reset the buffer
    logger.write(level, std::move(value));
    return this->cout_buffer.str();
  } };
  const auto test_regex { [](const std::string &test_patten, const std::string &regex_pattern) -> bool {
    std::regex regex(regex_pattern);
    std::smatch match;
    return std::regex_match(test_patten, match, regex);
  } };
  const auto add_os_newline { [](std::string value) {
#ifdef _WIN32
    value += "\r\n";
#else
    value += "\n";
#endif
    return value;
  } };

  logger.set_log_level(level::verbose);
  EXPECT_TRUE(test_regex(write_and_get_cout(level::verbose, "Hello World!"), add_os_newline(R"(\[\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d\] VERBOSE: Hello World!)")));
  EXPECT_TRUE(test_regex(write_and_get_cout(level::debug, "Hello World!"), add_os_newline(R"(\[\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d\] DEBUG:   Hello World!)")));
  EXPECT_TRUE(test_regex(write_and_get_cout(level::info, "Hello World!"), add_os_newline(R"(\[\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d\] INFO:    Hello World!)")));
  EXPECT_TRUE(test_regex(write_and_get_cout(level::error, "Hello World!"), add_os_newline(R"(\[\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d\] ERROR:   Hello World!)")));
}

TEST(LoggingTest, CustomCallback) {
  using level = display_device::logger_t::log_level_e;
  using level_t = std::underlying_type_t<level>;
  auto &logger { display_device::logger_t::get() };

  std::string output;
  logger.set_log_level(level::verbose);
  logger.set_custom_callback([&output](const level level, std::string value) {
    output = std::to_string(static_cast<level_t>(level)) + " " + value;
  });

  logger.write(level::verbose, "Hello World!");
  EXPECT_EQ(output, "0 Hello World!");
  EXPECT_TRUE(this->cout_buffer.str().empty());

  logger.write(level::debug, "Hello World!");
  EXPECT_EQ(output, "1 Hello World!");
  EXPECT_TRUE(this->cout_buffer.str().empty());

  logger.write(level::info, "Hello World!");
  EXPECT_EQ(output, "2 Hello World!");
  EXPECT_TRUE(this->cout_buffer.str().empty());

  logger.write(level::error, "Hello World!");
  EXPECT_EQ(output, "3 Hello World!");
  EXPECT_TRUE(this->cout_buffer.str().empty());
}

TEST(LoggingTest, WriteMethodRespectsLogLevelWhenUsingDefaultLogger) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  EXPECT_TRUE(this->cout_buffer.str().empty());

  logger.set_log_level(level::error);
  logger.write(level::info, "Hello World!");
  EXPECT_TRUE(this->cout_buffer.str().empty());

  logger.set_log_level(level::info);
  logger.write(level::info, "Hello World!");
  EXPECT_FALSE(this->cout_buffer.str().empty());
}

TEST(LoggingTest, WriteMethodRespectsLogLevelWhenUsingCustomCallback) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  bool callback_invoked { false };
  logger.set_custom_callback([&callback_invoked](const level, std::string) {
    callback_invoked = true;
  });

  logger.set_log_level(level::error);
  logger.write(level::info, "Hello World!");
  EXPECT_EQ(callback_invoked, false);

  logger.set_log_level(level::info);
  logger.write(level::info, "Hello World!");
  EXPECT_EQ(callback_invoked, true);
}

TEST(LoggingTest, LogMacroDisablesStreamChain) {
  using level = display_device::logger_t::log_level_e;
  auto &logger { display_device::logger_t::get() };

  bool output_logged { false };
  logger.set_custom_callback([&output_logged](const level, std::string) {
    output_logged = true;
  });

  bool some_function_invoked { false };
  const auto some_function { [&some_function_invoked]() {
    some_function_invoked = true;
    return "some string";
  } };

  logger.set_log_level(level::error);
  DD_LOG(info) << some_function();
  EXPECT_EQ(output_logged, false);
  EXPECT_EQ(some_function_invoked, false);

  logger.set_log_level(level::info);
  DD_LOG(info) << some_function();
  EXPECT_EQ(output_logged, true);
  EXPECT_EQ(some_function_invoked, true);
}
