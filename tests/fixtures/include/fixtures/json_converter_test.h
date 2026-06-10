#pragma once

// local includes
#include "display_device/json.h"
#include "fixtures.h"

class JsonConverterTest: public BaseTest {
public:
  template<class T>
  void executeTestCase(const T &input, const std::string &expected_string) const {
    bool success {false};
    const auto json_string {display_device::toJson(input, std::nullopt, &success)};
    EXPECT_TRUE(success);
    EXPECT_EQ(json_string, expected_string);

    std::string error_message {};
    T defaulted_input {};
    if (!display_device::fromJson(json_string, defaulted_input, &error_message)) {
      GTEST_FAIL() << error_message;
    }
    EXPECT_EQ(input, defaulted_input);
  }

  template<class T>
  void executeInvalidJsonTestCase() const {
    executeFromJsonFailureTestCase<T>("{");
  }

  template<class T>
  void executeFromJsonFailureTestCase(const std::string &input) const {
    const T original {};
    T output {original};
    std::string error_message {"unchanged"};

    EXPECT_FALSE(display_device::fromJson(input, output, &error_message));
    EXPECT_EQ(output, original);
    EXPECT_FALSE(error_message.empty());

    EXPECT_FALSE(display_device::fromJson(input, output, nullptr));
    EXPECT_EQ(output, original);
  }

  template<class T>
  void executeToJsonFailureTestCase(const T &input) const {
    bool success {true};

    EXPECT_FALSE(display_device::toJson(input, display_device::JSON_COMPACT, &success).empty());
    EXPECT_FALSE(success);
    EXPECT_FALSE(display_device::toJson(input, display_device::JSON_COMPACT, nullptr).empty());
  }
};
