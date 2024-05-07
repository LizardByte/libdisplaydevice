// header include
#include "utils.h"

// system includes
#include <iostream>
#include <regex>

bool
testRegex(const std::string &test_pattern, const std::string &regex_pattern) {
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

int
setEnv(const std::string &name, const std::string &value) {
#ifdef _WIN32
  return _putenv_s(name.c_str(), value.c_str());
#else
  return setenv(name.c_str(), value.c_str(), 1);
#endif
}
