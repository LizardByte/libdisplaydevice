// header include
#include "fixtures/testutils.h"

// system includes
#include <cstdlib>

// system includes
#include <iostream>
#include <regex>

bool
testRegex(const std::string &input, const std::string &pattern) {
  std::regex regex(pattern);
  std::smatch match;
  if (!std::regex_match(input, match, regex)) {
    std::cout << "Regex test failed:\n"
              << "    Input  : " << input << "\n"
              << "    Pattern: " << pattern << std::endl;
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

std::optional<std::string>
getEnv(const std::string &name) {
  if (const auto value { std::getenv(name.c_str()) }; value) {
    return std::string { value };
  }
  return std::nullopt;
}
