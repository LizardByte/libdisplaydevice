/**
 * @file src/macos/display_power.cpp
 * @brief Definitions for macOS display power management.
 */
// class header include
#include "display_device/macos/display_power.h"

// system includes
#include <algorithm>
#include <charconv>
#include <chrono>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <utility>

namespace display_device {
  namespace {
    using namespace std::chrono_literals;

    /**
     * @brief Default reason used for short display wake assertions.
     */
    constexpr std::string_view DISPLAY_DETECTION_REASON {"libdisplaydevice display detection"};

    /**
     * @brief Retry interval for waiting until a display becomes active.
     */
    constexpr auto DISPLAY_WAKE_RETRY_INTERVAL {100ms};

    /**
     * @brief Parse a CoreGraphics display id from a capture selector.
     * @param display_name Platform capture selector.
     * @return Parsed display id, or empty optional if the selector is not numeric.
     */
    [[nodiscard]] std::optional<MacDisplayId> parseDisplayId(const std::string_view display_name) {
      if (display_name.empty()) {
        return std::nullopt;
      }

      MacDisplayId display_id {};
      const auto *const begin {display_name.data()};
      const auto *const end {display_name.data() + display_name.size()};
      const auto [ptr, ec] {std::from_chars(begin, end, display_id)};
      if (ec != std::errc {} || ptr != end) {
        return std::nullopt;
      }

      return display_id;
    }

    /**
     * @brief Scoped macOS power assertion.
     */
    class MacPowerAssertionGuard: public DisplayPowerGuardInterface {
    public:
      /**
       * @brief Constructor.
       * @param m_api macOS API layer.
       * @param assertion_id Power assertion id to release.
       */
      MacPowerAssertionGuard(std::shared_ptr<MacApiLayerInterface> m_api, const MacPowerAssertionId assertion_id):
          m_m_api {std::move(m_api)},
          m_assertion_id {assertion_id} {}

      MacPowerAssertionGuard(const MacPowerAssertionGuard &) = delete;  ///< Copy constructor.
      MacPowerAssertionGuard &operator=(const MacPowerAssertionGuard &) = delete;  ///< Copy assignment operator.
      MacPowerAssertionGuard(MacPowerAssertionGuard &&) = delete;  ///< Move constructor.
      MacPowerAssertionGuard &operator=(MacPowerAssertionGuard &&) = delete;  ///< Move assignment operator.

      /**
       * @brief Destructor.
       */
      ~MacPowerAssertionGuard() override {
        static_cast<void>(m_m_api->releasePowerAssertion(m_assertion_id));
      }

    private:
      std::shared_ptr<MacApiLayerInterface> m_m_api;  ///< macOS API layer.
      MacPowerAssertionId m_assertion_id {};  ///< Power assertion id to release.
    };
  }  // namespace

  MacDisplayPower::MacDisplayPower(std::shared_ptr<MacApiLayerInterface> m_api):
      m_m_api {std::move(m_api)} {
    if (!m_m_api) {
      throw std::invalid_argument {"Nullptr provided for MacApiLayerInterface in MacDisplayPower!"};
    }
  }

  bool MacDisplayPower::wakeDisplay(const std::string &display_name, const std::chrono::milliseconds timeout) {
    if (hasRequiredActiveDisplay(display_name)) {
      return true;
    }

    const auto assertion_id {m_m_api->declareUserActivity(std::string {DISPLAY_DETECTION_REASON})};
    if (!assertion_id.has_value()) {
      return false;
    }

    const auto assertion_guard {std::make_unique<MacPowerAssertionGuard>(m_m_api, *assertion_id)};
    const auto deadline {std::chrono::steady_clock::now() + timeout};

    do {
      if (hasRequiredActiveDisplay(display_name)) {
        return true;
      }

      const auto now {std::chrono::steady_clock::now()};
      if (now >= deadline) {
        break;
      }

      std::this_thread::sleep_for(std::min(DISPLAY_WAKE_RETRY_INTERVAL, std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now)));
    } while (true);

    return hasRequiredActiveDisplay(display_name);
  }

  std::unique_ptr<DisplayPowerGuardInterface> MacDisplayPower::keepDisplayAwake(const std::string &reason) {
    const auto assertion_id {m_m_api->createDisplaySleepAssertion(reason)};
    if (!assertion_id.has_value()) {
      return nullptr;
    }

    return std::make_unique<MacPowerAssertionGuard>(m_m_api, *assertion_id);
  }

  bool MacDisplayPower::hasRequiredActiveDisplay(const std::string &display_name) const {
    const auto active_displays {m_m_api->getDisplayIds(MacQueryType::Active)};
    if (const auto display_id {parseDisplayId(display_name)}; display_id.has_value()) {
      return std::ranges::find(active_displays, *display_id) != std::end(active_displays);
    }

    return !active_displays.empty();
  }
}  // namespace display_device
