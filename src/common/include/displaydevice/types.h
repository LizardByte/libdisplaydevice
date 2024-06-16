#pragma once

// system includes
#include <optional>
#include <string>
#include <vector>

namespace display_device {
  /**
   * @brief The device's HDR state in the operating system.
   */
  enum class HdrState {
    Disabled,
    Enabled
  };

  /**
   * @brief Display's resolution.
   */
  struct Resolution {
    unsigned int m_width {};
    unsigned int m_height {};
  };

  /**
   * @brief An arbitrary point object.
   */
  struct Point {
    int m_x {};
    int m_y {};
  };

  /**
   * @brief Enumerated display device information.
   */
  struct EnumeratedDevice {
    /**
     * @brief Available information for the active display only.
     */
    struct Info {
      Resolution m_resolution {}; /**< Resolution of an active device. */
      float m_resolution_scale {}; /**< Resolution scaling of an active device. */
      float m_refresh_rate {}; /**< Refresh rate of an active device. */
      bool m_primary {}; /**< Indicates whether the device is a primary display. */
      Point m_origin_point {}; /**< A starting point of the display. */
      std::optional<HdrState> m_hdr_state {}; /**< HDR of an active device. */
    };

    std::string m_device_id {}; /**< A unique device ID used by this API to identify the device. */
    std::string m_display_name {}; /**< A logical name representing given by the OS for a display. */
    std::string m_friendly_name {}; /**< A human-readable name for the device. */
    std::optional<Info> m_info {}; /**< Additional information about an active display device. */
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
      VerifyOnly, /**< User has to make sure the display device is active, we will only verify. */
      EnsureActive, /**< Activate the device if needed. */
      EnsurePrimary, /**< Activate the device if needed and make it a primary display. */
      EnsureOnlyDisplay /**< Deactivate other displays and turn on the specified one only. */
    };

    std::string m_device_id {}; /**< Device id manually provided by the user via config. */
    DevicePreparation m_device_prep {}; /**< Instruction on how to prepare device. */
    std::optional<Resolution> m_resolution {}; /**< Resolution to configure. */
    std::optional<float> m_refresh_rate {}; /**< Refresh rate to configure. */
    std::optional<HdrState> m_hdr_state {}; /**< HDR state to configure (if supported by the display). */
  };
}  // namespace display_device
