/**
 * @file src/common/include/display_device/types.h
 * @brief Declarations for common display device types.
 */
#pragma once

// system includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace display_device {
  /**
   * @brief Transparent hash for string-keyed unordered containers.
   */
  struct StringHash {
    /**
     * @brief Enables heterogeneous lookup for compatible string key types.
     */
    using is_transparent = void;

    /**
     * @brief Hash a string view.
     * @param value Value to hash.
     * @returns Hash value.
     */
    [[nodiscard]] std::size_t operator()(const std::string_view value) const noexcept {
      return std::hash<std::string_view> {}(value);
    }
  };

  /**
   * @brief Ordered set keyed by strings with transparent comparisons.
   */
  using StringSet = std::set<std::string, std::less<>>;

  /**
   * @brief Ordered map keyed by strings with transparent comparisons.
   */
  template<typename T>
  using StringMap = std::map<std::string, T, std::less<>>;

  /**
   * @brief Unordered map keyed by strings with transparent comparisons.
   */
  template<typename T>
  using StringUnorderedMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;

  /**
   * @brief Unordered set keyed by strings with transparent comparisons.
   */
  using StringUnorderedSet = std::unordered_set<std::string, StringHash, std::equal_to<>>;

  /**
   * @brief The device's HDR state in the operating system.
   */
  enum class HdrState {
    Disabled,  ///< HDR is disabled
    Enabled  ///< HDR is enabled
  };

  /**
   * @brief Display's resolution.
   */
  struct Resolution {
    unsigned int m_width {};  ///< Width in pixels.
    unsigned int m_height {};  ///< Height in pixels.

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const Resolution &lhs, const Resolution &rhs) {
      return lhs.m_height == rhs.m_height && lhs.m_width == rhs.m_width;
    }
  };

  /**
   * @brief An arbitrary point object.
   */
  struct Point {
    int m_x {};  ///< Horizontal coordinate.
    int m_y {};  ///< Vertical coordinate.

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const Point &lhs, const Point &rhs) = default;
  };

  /**
   * @brief Floating point stored in a "numerator/denominator" form.
   */
  struct Rational {
    unsigned int m_numerator {};  ///< Numerator.
    unsigned int m_denominator {};  ///< Denominator.

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const Rational &lhs, const Rational &rhs) = default;
  };

  /**
   * @brief Floating point type.
   */
  using FloatingPoint = std::variant<double, Rational>;

  namespace detail {
    /**
     * @brief Fuzzy comparison for floating-point values.
     * @param lhs First value to compare.
     * @param rhs Second value to compare.
     * @returns True if the values are close enough to be treated as equal.
     */
    inline bool fuzzyCompare(const double lhs, const double rhs) {
      return std::abs(lhs - rhs) * 1000000000000. <= std::min(std::abs(lhs), std::abs(rhs));
    }

    /**
     * @brief Fuzzy comparison for floating-point variant values.
     * @param lhs First value to compare.
     * @param rhs Second value to compare.
     * @returns True if the values are close enough to be treated as equal.
     */
    inline bool fuzzyCompare(const FloatingPoint &lhs, const FloatingPoint &rhs) {
      if (lhs.index() == rhs.index()) {
        if (std::holds_alternative<double>(lhs)) {
          return fuzzyCompare(std::get<double>(lhs), std::get<double>(rhs));
        }
        return lhs == rhs;
      }
      return false;
    }
  }  // namespace detail

  /**
   * @brief Parsed EDID data.
   */
  struct EdidData {
    std::string m_manufacturer_id {};  ///< Three-letter manufacturer ID.
    std::string m_product_code {};  ///< Product code.
    std::uint32_t m_serial_number {};  ///< Serial number.

    /**
     * @brief Parse EDID data.
     * @param data Data to parse.
     * @return Parsed data or empty optional if failed to parse it.
     */
    static std::optional<EdidData> parse(const std::vector<std::byte> &data);

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const EdidData &lhs, const EdidData &rhs) = default;
  };

  /**
   * @brief Enumerated display device information.
   */
  struct EnumeratedDevice {
    /**
     * @brief Available information for the active display only.
     */
    struct Info {
      Resolution m_resolution {};  ///< Resolution of an active device.
      FloatingPoint m_resolution_scale {};  ///< Resolution scaling of an active device.
      FloatingPoint m_refresh_rate {};  ///< Refresh rate of an active device.
      bool m_primary {};  ///< Indicates whether the device is a primary display.
      Point m_origin_point {};  ///< A starting point of the display.
      std::optional<HdrState> m_hdr_state {};  ///< HDR of an active device.

      /**
       * @brief Comparator for strict equality.
       */
      friend bool operator==(const Info &lhs, const Info &rhs) {
        return lhs.m_resolution == rhs.m_resolution && detail::fuzzyCompare(lhs.m_resolution_scale, rhs.m_resolution_scale) &&
               detail::fuzzyCompare(lhs.m_refresh_rate, rhs.m_refresh_rate) && lhs.m_primary == rhs.m_primary &&
               lhs.m_origin_point == rhs.m_origin_point && lhs.m_hdr_state == rhs.m_hdr_state;
      }
    };

    std::string m_device_id {};  ///< A unique device ID used by this API to identify the device.
    std::string m_display_name {};  ///< A logical name representing given by the OS for a display.
    std::string m_friendly_name {};  ///< A human-readable name for the device.
    std::optional<EdidData> m_edid {};  ///< Some basic parsed EDID data.
    std::optional<Info> m_info {};  ///< Additional information about an active display device.

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const EnumeratedDevice &lhs, const EnumeratedDevice &rhs) = default;
  };

  /**
   * @brief A list of EnumeratedDevice objects.
   */
  using EnumeratedDeviceList = std::vector<EnumeratedDevice>;

  /**
   * @brief Configuration centered around a single display.
   *
   * Allows to easily configure the display without providing a complete configuration
   * for all of the system display devices.
   */
  struct SingleDisplayConfiguration {
    /**
     * @brief Enum detailing how to prepare the display device.
     */
    enum class DevicePreparation {
      VerifyOnly,  ///< User has to make sure the display device is active, we will only verify.
      EnsureActive,  ///< Activate the device if needed.
      EnsurePrimary,  ///< Activate the device if needed and make it a primary display.
      EnsureOnlyDisplay  ///< Deactivate other displays and turn on the specified one only.
    };

    std::string m_device_id {};  ///< Device to perform configuration for (can be empty if primary device should be used).
    DevicePreparation m_device_prep {};  ///< Instruction on how to prepare device.
    std::optional<Resolution> m_resolution {};  ///< Resolution to configure.
    std::optional<FloatingPoint> m_refresh_rate {};  ///< Refresh rate to configure.
    std::optional<HdrState> m_hdr_state {};  ///< HDR state to configure (if supported by the display).

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const SingleDisplayConfiguration &lhs, const SingleDisplayConfiguration &rhs) = default;
  };
}  // namespace display_device
