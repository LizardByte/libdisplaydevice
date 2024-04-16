#pragma once

// system includes
#include <functional>
#include <sstream>
#include <string>

namespace display_device {
  /**
   * @brief A singleton class for logging or re-routing logs.
   *
   * This class is not meant to be used directly (only for configuration).
   * Instead, the MACRO below should be used throughout the code for logging.
   *
   * @note A lazy-evaluated, correctly-destroyed, thread-safe singleton pattern is used here (https://stackoverflow.com/a/1008289).
   */
  class logger_t {
  public:
    /**
     * @brief Defines the possible log levels.
     * @note Level implicitly includes all other levels below it.
     * @note All levels are in lower-case on purpose to fit the "BOOST_LOG(info)" style.
     */
    enum class log_level_e {
      verbose = 0,
      debug,
      info,
      warning,
      error
    };

    /**
     * @brief Defines the callback type for log data re-routing.
     */
    using callback_t = std::function<void(const log_level_e, std::string)>;

    /**
     * @brief Get the singleton instance.
     * @returns Singleton instance for the class.
     *
     * EXAMPLES:
     * ```cpp
     * logger_t& logger { logger_t::get() };
     * ```
     */
    static logger_t &
    get();

    /**
     * @brief Set the log level for the logger.
     * @param log_level New level to be used.
     *
     * EXAMPLES:
     * ```cpp
     * logger_t::get().set_log_level(logger_t::log_level_e::Info);
     * ```
     */
    void
    set_log_level(const log_level_e log_level);

    /**
     * @brief Check if log level is currently enabled.
     * @param log_level Log level to check.
     * @returns True if log level is enabled.
     *
     * EXAMPLES:
     * ```cpp
     * const bool is_enabled { logger_t::get().is_log_level_enabled(logger_t::log_level_e::Info) };
     * ```
     */
    [[nodiscard]] bool
    is_log_level_enabled(const log_level_e log_level) const;

    /**
     * @brief Set custom callback for writing the logs.
     * @param callback New callback to be used or nullptr to reset to the default.
     *
     * EXAMPLES:
     * ```cpp
     * logger_t::get().set_custom_callback([](const log_level_e level, std::string value){
     *    // write to file or something
     * });
     * ```
     */
    void
    set_custom_callback(callback_t callback);

    /**
     * @brief Write the string to the output (via callback) if the log level is enabled.
     * @param log_level Log level to be checked and (probably) written.
     * @param value A copy of the string to be written.
     *
     * EXAMPLES:
     * ```cpp
     * logger_t::get().write(logger_t::log_level_e::Info, "Hello World!");
     * ```
     */
    void
    write(const log_level_e log_level, std::string value);

    /**
     * @brief A deleted copy constructor for singleton pattern.
     * @note Public to ensure better compiler error message.
     */
    logger_t(logger_t const &) = delete;

    /**
     * @brief A deleted assignment operator for singleton pattern.
     * @note Public to ensure better compiler error message.
     */
    void
    operator=(logger_t const &) = delete;

  private:
    /**
     * @brief A private constructor to ensure the singleton pattern.
     */
    explicit logger_t();

    log_level_e m_enabled_log_level; /**< The currently enabled log level. */
    callback_t m_custom_callback; /**< Custom callback to pass log data to. */
  };

  /**
   * @brief A helper class for accumulating output via the stream operator and then writing it out at once.
   */
  class log_writer_t {
  public:
    /**
     * @brief Constructor scoped writer utility.
     * @param log_level Level to be used when writing out the output.
     */
    explicit log_writer_t(const logger_t::log_level_e log_level);

    /**
     * @brief Write out the accumulated output.
     */
    virtual ~log_writer_t();

    /**
     * @brief Stream value to the buffer.
     * @param value Arbitrary value to be written to the buffer.
     * @returns Reference to the writer utility for chaining the operators.
     */
    template <class T>
    log_writer_t &
    operator<<(T &&value) {
      m_buffer << std::forward<T>(value);
      return *this;
    }

  private:
    logger_t::log_level_e m_log_level; /**< Log level to be used. */
    std::ostringstream m_buffer; /**< Buffer to hold all the output. */
  };
}  // namespace display_device

/**
 * @brief Helper MACRO that disables output string computation if log level is not enabled.
 *
 * EXAMPLES:
 * ```cpp
 * DD_LOG(info) << "Hello World!" << " " << 123;
 * DD_LOG(error) << "OH MY GAWD!";
 * ```
 */
#define DD_LOG(level)                                                                                                                                          \
  for (bool is_enabled { display_device::logger_t::get().is_log_level_enabled(display_device::logger_t::log_level_e::level) }; is_enabled; is_enabled = false) \
  display_device::log_writer_t(display_device::logger_t::log_level_e::level)
