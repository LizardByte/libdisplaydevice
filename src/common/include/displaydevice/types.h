#pragma once

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
    unsigned int m_width;
    unsigned int m_height;
  };

  /**
   * @brief Floating point stored in a "numerator/denominator" form.
   */
  struct Rational {
    unsigned int m_numerator;
    unsigned int m_denominator;
  };
}  // namespace display_device
