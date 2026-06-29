#pragma once

// system includes
#include <string>
#include <vector>

// local includes
#include "display_device/types.h"

/**
 * @brief Contains some useful predefined structures for UTs.
 * @note Data is to be extended with relevant information as needed.
 */
namespace ut_consts {
  extern const std::vector<std::byte> DEFAULT_EDID;
  extern const display_device::EdidData DEFAULT_EDID_DATA;
}  // namespace ut_consts

/**
 * @brief Test regular expression against string.
 * @return True if string matches the regex, false otherwise.
 */
bool testRegex(const std::string &test_pattern, const std::string &regex_pattern);
