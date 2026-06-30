#pragma once

// system includes
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <filesystem>

#define LIZARDBYTE_COMMON_TESTING_KEEP_GTEST_TEST
#define LIZARDBYTE_COMMON_TESTING_NO_GLOBAL_ALIASES
#include <lizardbyte/common/testing.h>

// local includes
#include "display_device/logging.h"
#include "test_utils.h"

// Undefine the original TEST macro
#undef TEST  // NOSONAR(cpp:S959): The test suite intentionally wraps TEST to use BaseTest.

// Redefine TEST to use our BaseTest class, to automatically use our BaseTest fixture
#define TEST(test_case_name, test_name) \
  GTEST_TEST_(test_case_name, test_name, ::BaseTest, ::testing::internal::GetTypeId<::BaseTest>())

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
 */
class BaseTest: public ::lizardbyte::common::testing::BaseTest {
protected:
  ~BaseTest() override = default;

  void SetUp() override;

  void TearDown() override;

  /**
   * @brief Get the default log level for the test base.
   * @returns A log level set in the env OR null optional if fallback should be used (verbose).
   * @note By setting LOG_LEVEL=<level> env you can change the level (e.g. LOG_LEVEL=error).
   */
  [[nodiscard]] virtual std::optional<display_device::Logger::LogLevel> getDefaultLogLevel() const;

private:
  bool m_test_skipped_at_setup {false}; /**< Indicates whether the SetUp method was skipped. */
};
