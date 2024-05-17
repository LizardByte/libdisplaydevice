#pragma once

// system includes
#include <optional>
#include <string>

/**
 * @brief Test regular expression against string.
 * @return True if string matches the regex, false otherwise.
 */
bool
testRegex(const std::string &test_pattern, const std::string &regex_pattern);

/**
 * @brief Set an environment variable.
 * @param name Name of the environment variable.
 * @param value Value of the environment variable.
 * @return 0 on success, non-zero error code on failure.
 */
int
setEnv(const std::string &name, const std::string &value);

/**
 * @brief Get an environment variable.
 * @param name Name of the environment variable.
 * @return String value of the variable or an empty optional otherwise.
 */
std::optional<std::string>
getEnv(const std::string &name);
