#pragma once

// system includes
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>

// local includes
#include "logging.h"

namespace display_device {
  /**
   * @brief A convenience class for stoping the RetryScheduler.
   *
   * It is conceptualy similar to `std::stop_token` except that it also used
   * RAII to perform a cleanup. This allows to allows to return void types
   * in the RetryScheduler without a hastle.
   */
  class SchedulerStopToken final {
  public:
    /**
     * @brief Default constructor.
     * @param cleanup Function to be executed once the destructor is called (object goes out of scope).
     */
    explicit SchedulerStopToken(std::function<void()> cleanup);

    /**
     * @brief Executes cleanup logic if scheduler stop was requested.
     */
    ~SchedulerStopToken();

    /**
     * @brief Request the scheduler to be stopped.
     */
    void
    requestStop();

    /**
     * @brief Check if stop was requested.
     * @return True if stop was requested, false otherwise.
     */
    [[nodiscard]] bool
    stopRequested() const;

  private:
    bool m_stop_requested { false };
    std::function<void()> m_cleanup;
  };

  namespace detail {
    /**
     * @brief Given that we know that we are dealing with a function,
     *        check if it is an optional function (like std::function<...>) or other callable.
     */
    template <class FunctionT>
    concept OptionalFunction = requires(FunctionT exec_fn) {
      static_cast<bool>(exec_fn);
    };

    /**
     * @brief Check if the function signature matches the acceptable signature for RetryScheduler::execute
     *        without a stop token.
     */
    template <class T, class FunctionT>
    concept ExecuteWithoutStopToken = requires(FunctionT exec_fn) {
      exec_fn(std::declval<T &>());
    };

    /**
     * @brief Check if the function signature matches the acceptable signature for RetryScheduler::execute
     *        with a stop token.
     */
    template <class T, class FunctionT>
    concept ExecuteWithStopToken = requires(FunctionT exec_fn) {
      exec_fn(std::declval<T &>(), std::declval<SchedulerStopToken &>());
    };

    /**
     * @brief Check if the function signature matches the acceptable signature for RetryScheduler::execute.
     */
    template <class T, class FunctionT>
    concept ExecuteCallbackLike = ExecuteWithoutStopToken<T, FunctionT> || ExecuteWithStopToken<T, FunctionT>;
  }  // namespace detail

  /**
   * @brief A wrapper class around an interface that provides a thread-safe access to the
   *        interface and allows to schedule arbitrary logic for it to retry until it succeeds.
   * @note The scheduler is designed to only schedule 1 callback at a time, until it is either
   *       replaced or stopped.
   */
  template <class T>
  class RetryScheduler final {
  public:
    /**
     * @bried Default constructor.
     * @param iface Interface to be passed around to the executor functions.
     */
    explicit RetryScheduler(std::unique_ptr<T> iface):
        m_iface { iface ? std::move(iface) : throw std::logic_error { "Nullptr interface provided in RetryScheduler!" } },
        m_thread { [this]() {
          std::unique_lock lock { m_mutex };
          while (m_keep_alive) {
            m_syncing_thread = false;
            if (m_sleep_duration > std::chrono::milliseconds::zero()) {
              // We're going to sleep until manually woken up or the time elapses.
              m_sleep_cv.wait_for(lock, m_sleep_duration, [this]() { return m_syncing_thread; });
            }
            else {
              // We're going to sleep until manually woken up.
              m_sleep_cv.wait(lock, [this]() { return m_syncing_thread; });
            }

            if (m_syncing_thread) {
              // Thread was waken up to sync sleep time or to be stopped.
              continue;
            }

            try {
              SchedulerStopToken scheduler_stop_token { [&]() { clearThreadLoopUnlocked(); } };
              m_retry_function(*m_iface, scheduler_stop_token);
              continue;
            }
            catch (const std::exception &error) {
              DD_LOG(error) << "Exception thrown in the RetryScheduler thread. Stopping scheduler. Error:\n"
                            << error.what();
            }

            clearThreadLoopUnlocked();
          }
        } } {
    }

    /**
     * @brief A destructor that gracefully shuts down the thread.
     */
    ~RetryScheduler() {
      {
        std::lock_guard lock { m_mutex };
        m_keep_alive = false;
        syncThreadUnlocked();
      }

      m_thread.join();
    }

    /**
     * @brief Schedule an interface executor function to be executed at specified intervals.
     * @param exec_fn Provides thread-safe access to the interface for executing arbitrary logic.
     *                It accepts a `stop_token` as a second parameter which can be used to stop
     *                the scheduler.
     * @param interval Specify the interval for the scheduler.
     * @note Before the executor function is scheduled, it is first executed in the calling thread
     *       immediately and the callback is invoked before returning!
     * @note Previously scheduled executor is replaced by a new one!
     *
     * EXAMPLES:
     * ```cpp
     * std::unique_ptr<SettingsManagerInterface> iface = getIface(...);
     * RetryScheduler<SettingsManagerInterface> scheduler{std::move(iface)};
     *
     * scheduler.schedule([](SettingsManagerInterface& iface, SchedulerStopToken& stop_token){
     *   if (iface.revertSettings()) {
     *     stop_token.requestStop();
     *   }
     * }, 5ms);
     * ```
     */
    void
    schedule(std::function<void(T &, SchedulerStopToken &stop_token)> exec_fn, std::chrono::milliseconds interval) {
      if (!exec_fn) {
        throw std::logic_error { "Empty callback function provided in RetryScheduler::schedule!" };
      }

      if (interval == std::chrono::milliseconds::zero()) {
        throw std::logic_error { "Interval cannot be zero in RetryScheduler::schedule!" };
      }

      std::lock_guard lock { m_mutex };
      SchedulerStopToken stop_token { [&]() { stopUnlocked(); } };

      // We are catching the exception here instead of propagating to have
      // similar try...catch login as in the scheduler thread.
      try {
        exec_fn(*m_iface, stop_token);
        if (!stop_token.stopRequested()) {
          m_retry_function = std::move(exec_fn);
          m_sleep_duration = interval;
          syncThreadUnlocked();
        }
      }
      catch (const std::exception &error) {
        stop_token.requestStop();
        DD_LOG(error) << "Exception thrown in the RetryScheduler::schedule. Stopping scheduler. Error:\n"
                      << error.what();
      }
    }

    /**
     * @brief Execute a arbitrary logic using the provided interface in a thread-safe manner.
     * @param exec_fn Provides thread-safe access to the interface for executing arbitrary logic.
     *                Acceptable function signatures are:
     *                  - AnyReturnType(T &);
     *                  - AnyReturnType(T &, SchedulerStopToken& stop_token),
     *                    `stop_token` is an optional parameter that allows to stop scheduler during
     *                    the same call.
     * @return Return value from the executor callback.
     *
     * EXAMPLES:
     * ```cpp
     * std::unique_ptr<SettingsManagerInterface> iface = getIface(...);
     * RetryScheduler<SettingsManagerInterface> scheduler{std::move(iface)};
     * const std::string device_id { "MY_DEVICE_ID" };
     *
     * // Without stop token:
     * const auto display_name = scheduler.execute([&](SettingsManagerInterface& iface) {
     *   return iface.getDisplayName(device_id);
     * });
     *
     * // With stop token:
     * scheduler.schedule([](SettingsManagerInterface& iface, SchedulerStopToken& stop_token) {
     *   if (iface.revertSettings()) {
     *     stop_token.requestStop();
     *   }
     * });
     *
     * scheduler.execute([&](SettingsManagerInterface& iface, SchedulerStopToken& stop_token) {
     *   if (true) {
     *     // Some condition was met and we no longer want to revert settings!
     *     stop_token.requestStop();
     *   }
     * });
     * ```
     */
    template <class FunctionT>
      requires detail::ExecuteCallbackLike<T, FunctionT>
    auto
    execute(FunctionT exec_fn) {
      if constexpr (detail::OptionalFunction<FunctionT>) {
        if (!exec_fn) {
          throw std::logic_error { "Empty callback function provided in RetryScheduler::execute!" };
        }
      }

      std::lock_guard lock { m_mutex };
      if constexpr (detail::ExecuteWithStopToken<T, FunctionT>) {
        SchedulerStopToken stop_token { [&]() { stopUnlocked(); } };
        return exec_fn(*m_iface, stop_token);
      }
      else {
        return exec_fn(*m_iface);
      }
    }

    /**
     * @brief Check whether anything is scheduled for execution.
     * @return True if something is scheduled, false otherwise.
     */
    [[nodiscard]] bool
    isScheduled() const {
      return static_cast<bool>(m_retry_function);
    }

    /**
     * @brief Stop the scheduled function - will no longer be execute once THIS method returns.
     */
    void
    stop() {
      std::lock_guard lock { m_mutex };
      stopUnlocked();
    }

  private:
    /**
     * @brief Clear the necessary data so that the thread will go into a deep sleep.
     */
    void
    clearThreadLoopUnlocked() {
      m_sleep_duration = std::chrono::milliseconds::zero();
      m_retry_function = nullptr;
    }

    /**
     * @brief Manually wake up the thread for synchronization.
     */
    void
    syncThreadUnlocked() {
      m_syncing_thread = true;
      m_sleep_cv.notify_one();
    }

    /**
     * @brief Stop the scheduled function.
     */
    void
    stopUnlocked() {
      if (isScheduled()) {
        clearThreadLoopUnlocked();
        syncThreadUnlocked();
      }
    }

    std::unique_ptr<T> m_iface; /**< Interface to be passed around to the executor functions. */
    std::chrono::milliseconds m_sleep_duration { 0 }; /**< A retry time for the timer. */
    std::function<void(T &, SchedulerStopToken &)> m_retry_function { nullptr }; /**< Function to be executed until it succeeds. */

    std::mutex m_mutex {}; /**< A mutext for synchronizing thread and "external" access. */
    std::condition_variable m_sleep_cv {}; /**< Condition variable for waking up thread. */
    bool m_syncing_thread { false }; /**< Safeguard for the condition variable to prevent sporadic thread wake ups. */
    bool m_keep_alive { true }; /**< When set to false, scheduler thread will exit. */

    // Always the last in the list so that all the members are already initialized!
    std::thread m_thread; /**< A scheduler thread. */
  };
}  // namespace display_device
