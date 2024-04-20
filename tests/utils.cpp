// header include
#include "utils.h"

// system includes
#include <iostream>

bool
test_regex(const std::string &test_pattern, const std::string &regex_pattern) {
  std::regex regex(regex_pattern);
  std::smatch match;
  if (!std::regex_match(test_pattern, match, regex)) {
    std::cout << "Regex test failed:\n"
              << "    Pattern: " << test_pattern << "\n"
              << "    Regex  : " << regex_pattern << std::endl;
    return false;
  }
  return true;
}
