#pragma once

// local includes
#include "comparison.h"
#include "displaydevice/json.h"
#include "fixtures.h"

class JsonConverterTest: public BaseTest {
public:
  template <class T>
  void
  executeTestCase(const T &input, const std::string &expected_string) {
    bool success { false };
    const auto json_string { display_device::toJson(input, std::nullopt, &success) };
    EXPECT_TRUE(success);
    EXPECT_EQ(json_string, expected_string);

    T defaulted_input {};
    EXPECT_TRUE(display_device::fromJson(json_string, defaulted_input));
    EXPECT_EQ(input, defaulted_input);
  }
};
