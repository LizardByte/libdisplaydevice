/**
 * @file src/windows/display_power.cpp
 * @brief Definitions for Windows display power management.
 */
// class header include
#include "display_device/windows/display_power.h"

// system includes
#include <stdexcept>
#include <utility>

namespace display_device {
  namespace {
    /**
     * @brief Scoped Windows display power request.
     */
    class WinDisplayPowerGuard: public DisplayPowerGuardInterface {
    public:
      /**
       * @brief Constructor.
       * @param w_api Windows API layer.
       */
      explicit WinDisplayPowerGuard(std::shared_ptr<WinApiLayerInterface> w_api):
          m_w_api {std::move(w_api)} {}

      WinDisplayPowerGuard(const WinDisplayPowerGuard &) = delete;  ///< Copy constructor.
      WinDisplayPowerGuard &operator=(const WinDisplayPowerGuard &) = delete;  ///< Copy assignment operator.
      WinDisplayPowerGuard(WinDisplayPowerGuard &&) = delete;  ///< Move constructor.
      WinDisplayPowerGuard &operator=(WinDisplayPowerGuard &&) = delete;  ///< Move assignment operator.

      /**
       * @brief Destructor.
       */
      ~WinDisplayPowerGuard() override {
        static_cast<void>(m_w_api->restorePowerRequest());
      }

    private:
      std::shared_ptr<WinApiLayerInterface> m_w_api;  ///< Windows API layer.
    };
  }  // namespace

  WinDisplayPower::WinDisplayPower(std::shared_ptr<WinApiLayerInterface> w_api):
      m_w_api {std::move(w_api)} {
    if (!m_w_api) {
      throw std::invalid_argument {"Nullptr provided for WinApiLayerInterface in WinDisplayPower!"};
    }
  }

  bool WinDisplayPower::wakeDisplay(const std::string &display_name, const std::chrono::milliseconds timeout) {
    static_cast<void>(display_name);
    return m_w_api->wakeDisplay(timeout);
  }

  std::unique_ptr<DisplayPowerGuardInterface> WinDisplayPower::keepDisplayAwake(const std::string &reason) {
    static_cast<void>(reason);
    if (!m_w_api->keepDisplayAwake()) {
      return nullptr;
    }

    return std::make_unique<WinDisplayPowerGuard>(m_w_api);
  }
}  // namespace display_device
