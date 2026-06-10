/**
 * @file src/common/include/display_device/file_settings_persistence.h
 * @brief Declarations for persistent file settings.
 */
#pragma once

// system includes
#include <filesystem>

// local includes
#include "settings_persistence_interface.h"

namespace display_device {
  /**
   * @brief Implementation of the SettingsPersistenceInterface,
   *        that saves/loads the persistent settings to/from the file.
   */
  class FileSettingsPersistence: public SettingsPersistenceInterface {
  public:
    /**
     * Default constructor. Does not perform any operations on the file yet.
     * @param filepath A non-empty filepath. Throws on empty.
     */
    explicit FileSettingsPersistence(std::filesystem::path filepath);

    /**
     * @copydoc SettingsPersistenceInterface::store
     * @warning The method does not create missing directories!
     */
    [[nodiscard]] bool store(const std::vector<std::uint8_t> &data) override;

    /**
     * @copydoc SettingsPersistenceInterface::load
     * @note If file does not exist, an empty data list will be returned instead of null optional.
     * @note If the path exists but is not a regular file, null optional will be returned.
     */
    [[nodiscard]] std::optional<std::vector<std::uint8_t>> load() const override;

    /**
     * @copydoc SettingsPersistenceInterface::clear
     */
    [[nodiscard]] bool clear() override;

  private:
    std::filesystem::path m_filepath;
  };
}  // namespace display_device
