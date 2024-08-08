// local includes
#include "display_device/windows/json.h"
#include "display_device/windows/settings_manager.h"
#include "display_device/windows/win_api_layer.h"
#include "display_device/windows/win_display_device.h"
#include "fixtures/fixtures.h"
#include "utils/guards.h"

namespace {
  // Convenience stuff for GTest
#define GTEST_DISABLED_CLASS_NAME(x) DISABLED_##x

  // Test fixture(s) for this file
  class GTEST_DISABLED_CLASS_NAME(WinPlayground):
      public BaseTest {
  public:
    bool
    isOutputSuppressed() const override {
      return false;
    }

    std::optional<display_device::Logger::LogLevel>
    getDefaultLogLevel() const override {
      // Unless user explicitly has overriden the level via ENV, we don't want all
      // that noise from verbose logs...
      return BaseTest::getDefaultLogLevel().value_or(display_device::Logger::LogLevel::info);
    }

    std::unique_ptr<display_device::SettingsManager> m_settings { std::make_unique<display_device::SettingsManager>(
      std::make_shared<display_device::WinDisplayDevice>(std::make_shared<display_device::WinApiLayer>()),
      nullptr,
      std::make_unique<display_device::PersistentState>(nullptr), display_device::WinWorkarounds {}) };
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, GTEST_DISABLED_CLASS_NAME(WinPlayground), __VA_ARGS__)
}  // namespace

TEST_F_S(EnumAvailableDevices) {
  // Usage example:
  //   test_libdisplaydevice.exe --gtest_color=yes --gtest_also_run_disabled_tests --gtest_filter=*WinPlayground.EnumAvailableDevices

  DD_LOG(info) << "enumerated devices:\n"
               << toJson(m_settings->enumAvailableDevices());
}

TEST_F_S(ApplySettings) {
  // Usage example:
  //   test_libdisplaydevice.exe --gtest_color=yes --gtest_also_run_disabled_tests --gtest_filter=*WinPlayground.ApplySettings config='{\"device_id\":\"{77f67f3e-754f-5d31-af64-ee037e18100a}\",\"device_prep\":\"EnsureActive\",\"hdr_state\":null,\"refresh_rate\":null,\"resolution\":null}'

  const auto arg { getArgWithMatchingPattern(R"(^config=)", true) };
  if (!arg) {
    GTEST_FAIL() << "\"config=<json_string>\" argument not found!";
  }

  std::string parse_error {};
  display_device::SingleDisplayConfiguration config;
  if (!fromJson(*arg, config, &parse_error)) {
    GTEST_FAIL() << "Config argument could not be parsed!\nArgument:\n  " << *arg << "\nError:\n  " << parse_error;
  }

  const boost::scope::scope_exit cleanup { [this]() { static_cast<void>(m_settings->revertSettings()); } };

  std::cout << "Applying settings. Press enter to continue..." << std::endl;
  std::cin.get();
  if (m_settings->applySettings(config) != display_device::SettingsManagerInterface::ApplyResult::Ok) {
    GTEST_FAIL() << "Failed to apply configuration!";
  }

  std::cout << "Reverting settings. Press enter to continue..." << std::endl;
  std::cin.get();
}
