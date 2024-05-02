#pragma once

// system includes
#include <filesystem>
#include <gtest/gtest.h>

// local includes
#include "utils.h"

// Undefine the original TEST macro
#undef TEST

// Redefine TEST to use our BaseTest class, to automatically use our BaseTest fixture
#define TEST(test_case_name, test_name)              \
  GTEST_TEST_(test_case_name, test_name, ::BaseTest, \
    ::testing::internal::GetTypeId<::BaseTest>())

/**
 * @brief Base class for tests.
 *
 * This class provides a base test fixture for all tests.
 *
 * ``cout``, ``stderr``, and ``stdout`` are redirected to a buffer, and the buffer is printed if the test fails.
 *
 * @todo Retain the color of the original output.
 */
class BaseTest: public ::testing::Test {
protected:
  // https://stackoverflow.com/a/58369622/11214013

  // we can possibly use some internal googletest functions to capture stdout and stderr, but I have not tested this
  // https://stackoverflow.com/a/33186201/11214013

  BaseTest();
  ~BaseTest() override = default;

  void
  SetUp() override;

  void
  TearDown() override;

  int
  exec(const char *cmd);

  // functions and variables
  std::vector<std::string> testArgs;  // CLI arguments used
  std::filesystem::path testBinary;  // full path of this binary
  std::filesystem::path testBinaryDir;  // full directory of this binary
  std::stringstream cout_buffer;  // declare cout_buffer
  std::stringstream stdout_buffer;  // declare stdout_buffer
  std::stringstream stderr_buffer;  // declare stderr_buffer
  std::streambuf *sbuf;
  FILE *pipe_stdout;
  FILE *pipe_stderr;
};

class LinuxTest: public BaseTest {
protected:
  void
  SetUp() override;

  void
  TearDown() override;
};

class MacOSTest: public BaseTest {
protected:
  void
  SetUp() override;

  void
  TearDown() override;
};

class WindowsTest: public BaseTest {
protected:
  void
  SetUp() override;

  void
  TearDown() override;
};
