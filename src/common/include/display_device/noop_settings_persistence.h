/**
 * @file src/common/include/display_device/noop_settings_persistence.h
 * @brief Declarations for NoopSettingsPersistence.
 */
#pragma once

// local includes
#include "settings_persistence_interface.h"

namespace display_device {
  /**
   * @brief A no-operation implementation for SettingsPersistenceInterface.
   */
  class NoopSettingsPersistence: public SettingsPersistenceInterface {
  public:
    /**
     * @copydoc SettingsPersistenceInterface::store
     */
    [[nodiscard]] bool store(const std::vector<std::uint8_t> &data) override;

    /**
     * @copydoc SettingsPersistenceInterface::load
     */
    [[nodiscard]] std::optional<std::vector<std::uint8_t>> load() const override;

    /**
     * @copydoc SettingsPersistenceInterface::clear
     */
    [[nodiscard]] bool clear() override;
  };
}  // namespace display_device
