/**
 * @file src/common/include/display_device/json.h
 * @brief Declarations for JSON conversion functions.
 */
#pragma once

// system includes
#include <set>

// local includes
#include "types.h"

/**
 * @brief Helper MACRO to declare the toJson and fromJson converters for a type.
 * @examples
 * EnumeratedDeviceList devices;
 * DD_LOG(info) << "Got devices:\n" << toJson(devices);
 * @examples_end
 */
#define DD_JSON_DECLARE_CONVERTER(Type) \
  [[nodiscard]] std::string toJson(const Type &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr); \
  [[nodiscard]] bool fromJson(const std::string &string, Type &obj, std::string *error_message = nullptr);  // NOLINT(*-macro-parentheses)

// Shared converters (add as needed)
namespace display_device {
  /**
   * @brief Indentation value for compact JSON output.
   */
  extern const std::optional<unsigned int> JSON_COMPACT;

  /**
   * @brief Serialize EDID data to JSON.
   * @param obj Object to serialize.
   * @param indent Optional indentation width. Use JSON_COMPACT for compact output.
   * @param success Optional output flag set to true on success, false otherwise.
   * @returns JSON string on success, error message otherwise.
   */
  [[nodiscard]] std::string toJson(const EdidData &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr);

  /**
   * @brief Deserialize EDID data from JSON.
   * @param string JSON string to parse.
   * @param obj Output object updated on success.
   * @param error_message Optional output error message.
   * @returns True on success, false otherwise.
   */
  [[nodiscard]] bool fromJson(const std::string &string, EdidData &obj, std::string *error_message = nullptr);

  /**
   * @brief Serialize an enumerated device to JSON.
   * @param obj Object to serialize.
   * @param indent Optional indentation width. Use JSON_COMPACT for compact output.
   * @param success Optional output flag set to true on success, false otherwise.
   * @returns JSON string on success, error message otherwise.
   */
  [[nodiscard]] std::string toJson(const EnumeratedDevice &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr);

  /**
   * @brief Deserialize an enumerated device from JSON.
   * @param string JSON string to parse.
   * @param obj Output object updated on success.
   * @param error_message Optional output error message.
   * @returns True on success, false otherwise.
   */
  [[nodiscard]] bool fromJson(const std::string &string, EnumeratedDevice &obj, std::string *error_message = nullptr);

  /**
   * @brief Serialize an enumerated device list to JSON.
   * @param obj Object to serialize.
   * @param indent Optional indentation width. Use JSON_COMPACT for compact output.
   * @param success Optional output flag set to true on success, false otherwise.
   * @returns JSON string on success, error message otherwise.
   */
  [[nodiscard]] std::string toJson(const EnumeratedDeviceList &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr);

  /**
   * @brief Deserialize an enumerated device list from JSON.
   * @param string JSON string to parse.
   * @param obj Output object updated on success.
   * @param error_message Optional output error message.
   * @returns True on success, false otherwise.
   */
  [[nodiscard]] bool fromJson(const std::string &string, EnumeratedDeviceList &obj, std::string *error_message = nullptr);

  /**
   * @brief Serialize a single display configuration to JSON.
   * @param obj Object to serialize.
   * @param indent Optional indentation width. Use JSON_COMPACT for compact output.
   * @param success Optional output flag set to true on success, false otherwise.
   * @returns JSON string on success, error message otherwise.
   */
  [[nodiscard]] std::string toJson(const SingleDisplayConfiguration &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr);

  /**
   * @brief Deserialize a single display configuration from JSON.
   * @param string JSON string to parse.
   * @param obj Output object updated on success.
   * @param error_message Optional output error message.
   * @returns True on success, false otherwise.
   */
  [[nodiscard]] bool fromJson(const std::string &string, SingleDisplayConfiguration &obj, std::string *error_message = nullptr);

  /**
   * @brief Serialize a string set to JSON.
   * @param obj Object to serialize.
   * @param indent Optional indentation width. Use JSON_COMPACT for compact output.
   * @param success Optional output flag set to true on success, false otherwise.
   * @returns JSON string on success, error message otherwise.
   */
  [[nodiscard]] std::string toJson(const StringSet &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr);

  /**
   * @brief Deserialize a string set from JSON.
   * @param string JSON string to parse.
   * @param obj Output object updated on success.
   * @param error_message Optional output error message.
   * @returns True on success, false otherwise.
   */
  [[nodiscard]] bool fromJson(const std::string &string, StringSet &obj, std::string *error_message = nullptr);

  /**
   * @brief Serialize a string to JSON.
   * @param obj Object to serialize.
   * @param indent Optional indentation width. Use JSON_COMPACT for compact output.
   * @param success Optional output flag set to true on success, false otherwise.
   * @returns JSON string on success, error message otherwise.
   */
  [[nodiscard]] std::string toJson(const std::string &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr);

  /**
   * @brief Deserialize a string from JSON.
   * @param string JSON string to parse.
   * @param obj Output object updated on success.
   * @param error_message Optional output error message.
   * @returns True on success, false otherwise.
   */
  [[nodiscard]] bool fromJson(const std::string &string, std::string &obj, std::string *error_message = nullptr);

  /**
   * @brief Serialize a boolean to JSON.
   * @param obj Object to serialize.
   * @param indent Optional indentation width. Use JSON_COMPACT for compact output.
   * @param success Optional output flag set to true on success, false otherwise.
   * @returns JSON string on success, error message otherwise.
   */
  [[nodiscard]] std::string toJson(const bool &obj, const std::optional<unsigned int> &indent = 2u, bool *success = nullptr);

  /**
   * @brief Deserialize a boolean from JSON.
   * @param string JSON string to parse.
   * @param obj Output object updated on success.
   * @param error_message Optional output error message.
   * @returns True on success, false otherwise.
   */
  [[nodiscard]] bool fromJson(const std::string &string, bool &obj, std::string *error_message = nullptr);
}  // namespace display_device
