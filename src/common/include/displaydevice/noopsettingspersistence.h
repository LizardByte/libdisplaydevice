#pragma once

// local includes
#include "settingspersistenceinterface.h"

namespace display_device {
  /**
   * @brief A no-operation implementation for SettingsPersistenceInterface.
   */
  class NoopSettingsPersistence: public SettingsPersistenceInterface {
  public:
    /** Always returns true. */
    [[nodiscard]] bool
    store(const std::vector<std::uint8_t> &) override;

    /** Always returns empty vector. */
    [[nodiscard]] std::optional<std::vector<std::uint8_t>>
    load() const override;

    /** Does nothing. */
    void
    clear() override;
  };
}  // namespace display_device
