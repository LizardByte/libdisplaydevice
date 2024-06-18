#pragma once

// system includes
#include <cstdint>
#include <optional>
#include <vector>

namespace display_device {
  /**
   * @brief A class for storing and loading settings data from a persistent medium.
   */
  class SettingsPersistenceInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~SettingsPersistenceInterface() = default;

    /**
     * @brief Store the provided data.
     * @param data Data array to store.
     * @returns True on success, false otherwise.
     *
     * EXAMPLES:
     * ```cpp
     * std::vector<std::uint8_t> data;
     * SettingsPersistenceInterface* iface = getIface(...);
     * const auto result = iface->store(data);
     * ```
     */
    [[nodiscard]] virtual bool
    store(const std::vector<std::uint8_t> &data) = 0;

    /**
     * @brief Load saved settings data.
     * @returns Null optional if failed to load data.
     *          Empty array, if there is no data.
     *          Non-empty array, if some data was loaded.
     *
     * EXAMPLES:
     * ```cpp
     * const SettingsPersistenceInterface* iface = getIface(...);
     * const auto opt_data = iface->load();
     * ```
     */
    [[nodiscard]] virtual std::optional<std::vector<std::uint8_t>>
    load() const = 0;

    /**
     * @brief Clear the persistent settings data.
     *
     * EXAMPLES:
     * ```cpp
     * SettingsPersistenceInterface* iface = getIface(...);
     * const auto result = iface->clear();
     * ```
     */
    [[nodiscard]] virtual bool
    clear() = 0;
  };
}  // namespace display_device
