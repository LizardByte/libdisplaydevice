// local includes
#include "displaydevice/noopaudiocontext.h"
#include "displaydevice/noopsettingspersistence.h"
#include "displaydevice/windows/settingsmanager.h"
#include "fixtures/fixtures.h"
#include "fixtures/mockaudiocontext.h"
#include "fixtures/mocksettingspersistence.h"
#include "utils/comparison.h"
#include "utils/mockwindisplaydevice.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::_;
  using ::testing::HasSubstr;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class SettingsManagerGeneralMocked: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockWinDisplayDevice>> m_dd_api { std::make_shared<StrictMock<display_device::MockWinDisplayDevice>>() };
    std::shared_ptr<StrictMock<display_device::MockSettingsPersistence>> m_settings_persistence_api { std::make_shared<StrictMock<display_device::MockSettingsPersistence>>() };
    std::shared_ptr<StrictMock<display_device::MockAudioContext>> m_audio_context_api { std::make_shared<StrictMock<display_device::MockAudioContext>>() };
    display_device::SettingsManager m_impl { m_dd_api, m_settings_persistence_api, m_audio_context_api };
  };

  // Specialized TEST macro(s) for this test SettingsManagerGeneralMocked
#define TEST_F_S_MOCKED(...) DD_MAKE_TEST(TEST_F, SettingsManagerGeneralMocked, __VA_ARGS__)
}  // namespace

TEST_F_S_MOCKED(NullptrDisplayDeviceApuProvided) {
  EXPECT_THAT([]() { const display_device::SettingsManager settings_manager(nullptr, nullptr, nullptr); },
    ThrowsMessage<std::logic_error>(HasSubstr("Nullptr provided for WinDisplayDeviceInterface in SettingsManager!")));
}

TEST_F_S_MOCKED(NoopAudioAndSettingsContext) {
  class NakedSettingsManager: public display_device::SettingsManager {
  public:
    using display_device::SettingsManager::SettingsManager;

    using display_device::SettingsManager::m_audio_context_api;
    using display_device::SettingsManager::m_settings_persistence_api;
  };

  const NakedSettingsManager settings_manager { std::make_shared<StrictMock<display_device::MockWinDisplayDevice>>(), nullptr, nullptr };
  EXPECT_TRUE(std::dynamic_pointer_cast<display_device::NoopAudioContext>(settings_manager.m_audio_context_api) != nullptr);
  EXPECT_TRUE(std::dynamic_pointer_cast<display_device::NoopSettingsPersistence>(settings_manager.m_settings_persistence_api) != nullptr);
}

TEST_F_S_MOCKED(EnumAvailableDevices) {
  const display_device::EnumeratedDeviceList test_list {
    { "DeviceId1",
      "",
      "FriendlyName1",
      std::nullopt }
  };

  EXPECT_CALL(*m_dd_api, enumAvailableDevices())
    .Times(1)
    .WillOnce(Return(test_list));

  EXPECT_EQ(m_impl.enumAvailableDevices(), test_list);
}

TEST_F_S_MOCKED(GetDisplayName) {
  EXPECT_CALL(*m_dd_api, getDisplayName("DeviceId1"))
    .Times(1)
    .WillOnce(Return("DeviceName1"));

  EXPECT_EQ(m_impl.getDisplayName("DeviceId1"), "DeviceName1");
}
