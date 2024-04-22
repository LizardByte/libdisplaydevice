#pragma once

// system includes
#include <string>

/**
 * @brief Test regular expression against string.
 * @return True if string matches the regex, false otherwise
 */
bool
test_regex(const std::string &test_pattern, const std::string &regex_pattern);

/**
 * @brief Set an environment variable.
 * @param name Name of the environment variable
 * @param value Value of the environment variable
 * @return 0 on success, non-zero error code on failure
 */
int
set_env(const std::string &name, const std::string &value);
