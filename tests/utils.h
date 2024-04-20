#pragma once

// system includes
#include <regex>
#include <string>

/**
 * @brief Test regular expression against string.
 * @return True if string matches the regex, false otherwise
 */
bool
test_regex(const std::string &test_pattern, const std::string &regex_pattern);
