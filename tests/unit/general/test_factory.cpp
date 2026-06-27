// system includes
#include <chrono>

// local includes
#include "display_device/factory.h"
#include "fixtures/fixtures.h"

namespace {
  using namespace std::chrono_literals;

  // Specialized TEST macro(s) for this test file
#define TEST_S(...) DD_MAKE_TEST(TEST, Factory, __VA_ARGS__)
}  // namespace

TEST_S(MakeSettingsManager) {
  display_device::SettingsManagerFactoryConfig config {
    .m_hdr_blank_delay = 10ms
  };

  const auto settings_manager {display_device::makeSettingsManager(config)};

#if defined(_WIN32) || defined(__APPLE__)
  ASSERT_NE(settings_manager, nullptr);
  EXPECT_TRUE(settings_manager->resetPersistence());
#else
  EXPECT_EQ(settings_manager, nullptr);
#endif
}

TEST_S(MakeDisplayPower) {
  const auto display_power {display_device::makeDisplayPower()};

#if defined(_WIN32) || defined(__APPLE__)
  EXPECT_NE(display_power, nullptr);
#else
  EXPECT_EQ(display_power, nullptr);
#endif
}
