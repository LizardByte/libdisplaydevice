#pragma once

// system includes
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <filesystem>
#include <gtest/gtest.h>

// local includes
#include "testutils.h"

// Undefine the original TEST macro
#undef TEST

// Redefine TEST to use our BaseTest class, to automatically use our BaseTest fixture
#define TEST(test_case_name, test_name)              \
  GTEST_TEST_(test_case_name, test_name, ::BaseTest, \
    ::testing::internal::GetTypeId<::BaseTest>())

// Helper macros for concatenating macro variables with an underscore separator (https://stackoverflow.com/questions/74505380/how-to-concatenate-join-va-args-with-delimiters-separators)
#define DD_CAT_OP_(index, state, elem) BOOST_PP_CAT(state, BOOST_PP_CAT(_, elem))
#define DD_CAT_SEQ_(seq) BOOST_PP_SEQ_FOLD_LEFT(DD_CAT_OP_, BOOST_PP_SEQ_HEAD(seq), BOOST_PP_SEQ_TAIL(seq))
#define DD_CAT_VA_MULTIPLE_(...) DD_CAT_SEQ_(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
#define DD_CAT_VA_SINGLE_(...) __VA_ARGS__
#define DD_CAT_VA_ARGS_(...) BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 1), DD_CAT_VA_SINGLE_, DD_CAT_VA_MULTIPLE_)(__VA_ARGS__)

// A macro for making the actual test macro.
// Usage example:
//   For normal tests:
//    #define TEST_S(...) DD_MAKE_TEST(TEST, SomeTestSuite, __VA_ARGS__)
//   For tests with fixtures:
//    #define TEST_F_S(...) DD_MAKE_TEST(TEST_F, SomeTestFixture, __VA_ARGS__)
#define DD_MAKE_TEST(test_macro, test_suite_name, ...) test_macro(test_suite_name, DD_CAT_VA_ARGS_(__VA_ARGS__))

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

  /**
   * @brief Check if the test interacts/modifies with the system settings.
   * @returns True if it does, false otherwise.
   * @note By setting SKIP_SYSTEM_TESTS=1 env, these tests will be skipped (useful during development).
   */
  [[nodiscard]] virtual bool
  isSystemTest() const;

  /**
   * @brief Skip the test by specifying the reason.
   * @returns A non-empty string (reason) if test needs to be skipped, empty string otherwise.
   */
  [[nodiscard]] virtual std::string
  skipTest() const;

  int
  exec(const char *cmd);

  // functions and variables
  std::vector<std::string> m_test_args;  // CLI arguments used
  std::filesystem::path m_test_binary;  // full path of this binary
  std::filesystem::path m_test_binary_dir;  // full directory of this binary
  std::stringstream m_cout_buffer;  // declare cout_buffer
  std::stringstream m_stdout_buffer;  // declare stdout_buffer
  std::stringstream m_stderr_buffer;  // declare stderr_buffer
  std::streambuf *m_sbuf;
  FILE *m_pipe_stdout;
  FILE *m_pipe_stderr;

private:
  bool m_test_skipped_at_setup { false };
};

class LinuxTest: public BaseTest {
protected:
  [[nodiscard]] std::string
  skipTest() const override;
};

class MacOSTest: public BaseTest {
protected:
  [[nodiscard]] std::string
  skipTest() const override;
};

class WindowsTest: public BaseTest {
protected:
  [[nodiscard]] std::string
  skipTest() const override;
};
