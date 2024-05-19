#pragma once

namespace display_device {
  /**
   * @brief Display's resolution.
   */
  struct Resolution {
    unsigned int m_width;
    unsigned int m_height;
  };

  /**
   * @brief Floating point is stored in a "numerator/denominator" form.
   */
  struct Rational {
    unsigned int m_numerator;
    unsigned int m_denominator;
  };
}  // namespace display_device
