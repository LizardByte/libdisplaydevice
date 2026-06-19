// system includes
#include <ranges>
#include <stdexcept>
#include <utility>

// local includes
#include "display_device/macos/mac_api_layer.h"
#include "display_device/macos/mac_api_utils.h"
#include "display_device/macos/mac_display_device.h"
#include "fixtures/fixtures.h"
#include "fixtures/test_utils.h"
#include "utils/mock_mac_api_layer.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::HasSubstr;
  using ::testing::InSequence;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class MacDisplayDeviceMocked: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockMacApiLayer>> m_layer {std::make_shared<StrictMock<display_device::MockMacApiLayer>>()};
    display_device::MacDisplayDevice m_mac_dd {m_layer};
  };

  class MacDisplayDeviceSystem: public BaseTest {
  public:
    bool isSystemTest() const override {
      return true;
    }

    std::shared_ptr<display_device::MacApiLayer> m_layer {std::make_shared<display_device::MacApiLayer>()};
    display_device::MacDisplayDevice m_mac_dd {m_layer};
  };

  /**
   * @brief Guard for restoring macOS display modes in live tests.
   */
  class MacModeGuard {
  public:
    /**
     * @brief Constructor.
     * @param mac_dd Display-device API to use for restoration.
     * @param modes Display modes to restore.
     */
    explicit MacModeGuard(display_device::MacDisplayDevice &mac_dd, display_device::MacDeviceDisplayModeMap modes):
        m_mac_dd {mac_dd},
        m_modes {std::move(modes)} {}

    /**
     * @brief Destructor.
     */
    ~MacModeGuard() {
      static_cast<void>(m_mac_dd.setDisplayModes(m_modes));
    }

  private:
    display_device::MacDisplayDevice &m_mac_dd;  ///< Display-device API.
    display_device::MacDeviceDisplayModeMap m_modes;  ///< Modes to restore.
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, MacDisplayDeviceMocked, __VA_ARGS__)
#define TEST_F_S_SYSTEM(...) DD_MAKE_TEST(TEST_F, MacDisplayDeviceSystem, __VA_ARGS__)
}  // namespace

TEST_F_S(NullptrLayerProvided) {
  EXPECT_THAT([]() {
    const auto mac_dd {display_device::MacDisplayDevice {nullptr}};
  },
              ThrowsMessage<std::invalid_argument>(HasSubstr("Nullptr provided for MacApiLayerInterface in MacDisplayDevice!")));
}

TEST_F_S(IsApiAccessAvailable) {
  EXPECT_CALL(*m_layer, isApiAccessAvailable())
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_FALSE(m_mac_dd.isApiAccessAvailable());
}

TEST_F_S(EnumAvailableDevices) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Online))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1, 2}));

  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getDeviceId(2))
    .Times(1)
    .WillOnce(Return("DeviceId2"));

  EXPECT_CALL(*m_layer, getDisplayName(1))
    .Times(1)
    .WillOnce(Return("DisplayName1"));
  EXPECT_CALL(*m_layer, getDisplayName(2))
    .Times(1)
    .WillOnce(Return("DisplayName2"));

  EXPECT_CALL(*m_layer, getFriendlyName(1))
    .Times(1)
    .WillOnce(Return("FriendlyName1"));
  EXPECT_CALL(*m_layer, getFriendlyName(2))
    .Times(1)
    .WillOnce(Return(""));

  EXPECT_CALL(*m_layer, getEdid(1))
    .Times(1)
    .WillOnce(Return(ut_consts::DEFAULT_EDID));
  EXPECT_CALL(*m_layer, getEdid(2))
    .Times(1)
    .WillOnce(Return(std::vector<std::byte> {}));

  EXPECT_CALL(*m_layer, isActive(1))
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_layer, isActive(2))
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));
  EXPECT_CALL(*m_layer, getDisplayScale(1))
    .Times(1)
    .WillOnce(Return(display_device::Rational {200, 100}));
  EXPECT_CALL(*m_layer, isMainDisplay(1))
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_layer, getOriginPoint(1))
    .Times(1)
    .WillOnce(Return(display_device::Point {0, 0}));

  const display_device::EnumeratedDeviceList expected_list {
    {"DeviceId1",
     "DisplayName1",
     "FriendlyName1",
     ut_consts::DEFAULT_EDID_DATA,
     display_device::EnumeratedDevice::Info {
       {1920, 1080},
       display_device::Rational {200, 100},
       display_device::Rational {60, 1},
       true,
       {0, 0},
       std::nullopt
     }},
    {"DeviceId2",
     "DisplayName2",
     "DisplayName2",
     std::nullopt,
     std::nullopt}
  };
  EXPECT_EQ(m_mac_dd.enumAvailableDevices(), expected_list);
}

TEST_F_S(GetDisplayName) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Online))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1, 2}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getDeviceId(2))
    .Times(1)
    .WillOnce(Return("DeviceId2"));
  EXPECT_CALL(*m_layer, getDisplayName(2))
    .Times(1)
    .WillOnce(Return("DisplayName2"));

  EXPECT_EQ(m_mac_dd.getDisplayName("DeviceId2"), "DisplayName2");
}

TEST_F_S(TopologyRead) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1, 2, 3}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getDeviceId(2))
    .Times(1)
    .WillOnce(Return("DeviceId2"));
  EXPECT_CALL(*m_layer, getDeviceId(3))
    .Times(1)
    .WillOnce(Return("DeviceId3"));
  EXPECT_CALL(*m_layer, getMirrorMaster(1))
    .Times(1)
    .WillOnce(Return(0));
  EXPECT_CALL(*m_layer, getMirrorMaster(2))
    .Times(1)
    .WillOnce(Return(1));
  EXPECT_CALL(*m_layer, getMirrorMaster(3))
    .Times(1)
    .WillOnce(Return(0));

  const display_device::MacActiveTopology expected_topology {{"DeviceId1", "DeviceId2"}, {"DeviceId3"}};
  EXPECT_EQ(m_mac_dd.getCurrentTopology(), expected_topology);
}

TEST_F_S(TopologyWriteStub) {
  EXPECT_FALSE(m_mac_dd.setTopology({{"DeviceId1"}}));
}

TEST_F_S(IsTopologyValid) {
  EXPECT_FALSE(m_mac_dd.isTopologyValid({/* no groups */}));
  EXPECT_FALSE(m_mac_dd.isTopologyValid({{/* empty group */}}));
  EXPECT_FALSE(m_mac_dd.isTopologyValid({{""}}));
  EXPECT_TRUE(m_mac_dd.isTopologyValid({{"ID_1"}}));
  EXPECT_TRUE(m_mac_dd.isTopologyValid({{"ID_1"}, {"ID_2"}}));
  EXPECT_TRUE(m_mac_dd.isTopologyValid({{"ID_1", "ID_2"}}));
  EXPECT_FALSE(m_mac_dd.isTopologyValid({{"ID_1", "ID_1"}}));
  EXPECT_FALSE(m_mac_dd.isTopologyValid({{"ID_1"}, {"ID_1"}}));
}

TEST_F_S(IsTopologyTheSame) {
  EXPECT_TRUE(m_mac_dd.isTopologyTheSame({/* no groups */}, {/* no groups */}));
  EXPECT_TRUE(m_mac_dd.isTopologyTheSame({{"ID_1"}}, {{"ID_1"}}));
  EXPECT_FALSE(m_mac_dd.isTopologyTheSame({{"ID_1"}}, {{"ID_1"}, {"ID_2"}}));
  EXPECT_TRUE(m_mac_dd.isTopologyTheSame({{"ID_1"}, {"ID_2"}}, {{"ID_2"}, {"ID_1"}}));
  EXPECT_FALSE(m_mac_dd.isTopologyTheSame({{"ID_1"}, {"ID_2"}}, {{"ID_1", "ID_2"}}));
  EXPECT_TRUE(m_mac_dd.isTopologyTheSame({{"ID_3"}, {"ID_1", "ID_2"}}, {{"ID_2", "ID_1"}, {"ID_3"}}));
}

TEST_F_S(GetCurrentDisplayModes) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(2)
    .WillRepeatedly(Return(display_device::MacDisplayIdList {1, 2}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(2)
    .WillRepeatedly(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getDeviceId(2))
    .Times(1)
    .WillOnce(Return("DeviceId2"));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(2))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{2560, 1440}, {120, 1}}));

  const display_device::MacDeviceDisplayModeMap expected_modes {
    {"DeviceId1", {{1920, 1080}, {60, 1}}},
    {"DeviceId2", {{2560, 1440}, {120, 1}}}
  };
  EXPECT_EQ(m_mac_dd.getCurrentDisplayModes({"DeviceId1", "DeviceId2"}), expected_modes);
}

TEST_F_S(SetDisplayModes, EmptyModes) {
  EXPECT_FALSE(m_mac_dd.setDisplayModes({}));
}

TEST_F_S(SetDisplayModes, InactiveDisplay) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId2"));

  EXPECT_FALSE(m_mac_dd.setDisplayModes({{"DeviceId1", {{1920, 1080}, {60, 1}}}}));
}

TEST_F_S(SetDisplayModes, UnavailableMode) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));
  EXPECT_CALL(*m_layer, getDisplayModes(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayModeList {{{1280, 720}, {60, 1}}}));

  EXPECT_FALSE(m_mac_dd.setDisplayModes({{"DeviceId1", {{1024, 768}, {60, 1}}}}));
}

TEST_F_S(SetDisplayModes, AlreadyCurrent) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));

  EXPECT_TRUE(m_mac_dd.setDisplayModes({{"DeviceId1", {{1920, 1080}, {5985, 100}}}}));
}

TEST_F_S(SetDisplayModes, Success) {
  InSequence sequence;

  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));
  EXPECT_CALL(*m_layer, getDisplayModes(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayModeList {{{1280, 720}, {60, 1}}}));
  EXPECT_CALL(*m_layer, setDisplayMode(1, display_device::MacDisplayMode {{1280, 720}, {60, 1}}))
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1280, 720}, {60, 1}}));

  EXPECT_TRUE(m_mac_dd.setDisplayModes({{"DeviceId1", {{1280, 720}, {60, 1}}}}));
}

TEST_F_S(SetDisplayModes, ApplyFailed) {
  InSequence sequence;

  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));
  EXPECT_CALL(*m_layer, getDisplayModes(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayModeList {{{1280, 720}, {60, 1}}}));
  EXPECT_CALL(*m_layer, setDisplayMode(1, display_device::MacDisplayMode {{1280, 720}, {60, 1}}))
    .Times(1)
    .WillOnce(Return(false));

  EXPECT_FALSE(m_mac_dd.setDisplayModes({{"DeviceId1", {{1280, 720}, {60, 1}}}}));
}

TEST_F_S(SetDisplayModes, VerificationFailedRollsBack) {
  InSequence sequence;

  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));
  EXPECT_CALL(*m_layer, getDisplayModes(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayModeList {{{1280, 720}, {60, 1}}, {{1920, 1080}, {60, 1}}}));
  EXPECT_CALL(*m_layer, setDisplayMode(1, display_device::MacDisplayMode {{1280, 720}, {60, 1}}))
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1024, 768}, {60, 1}}));
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1024, 768}, {60, 1}}));
  EXPECT_CALL(*m_layer, getDisplayModes(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayModeList {{{1920, 1080}, {60, 1}}}));
  EXPECT_CALL(*m_layer, setDisplayMode(1, display_device::MacDisplayMode {{1920, 1080}, {60, 1}}))
    .Times(1)
    .WillOnce(Return(true));
  EXPECT_CALL(*m_layer, getCurrentDisplayMode(1))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayMode {{1920, 1080}, {60, 1}}));

  EXPECT_FALSE(m_mac_dd.setDisplayModes({{"DeviceId1", {{1280, 720}, {60, 1}}}}));
}

TEST_F_S(IsPrimary) {
  EXPECT_CALL(*m_layer, getDisplayIds(display_device::MacQueryType::Active))
    .Times(1)
    .WillOnce(Return(display_device::MacDisplayIdList {1}));
  EXPECT_CALL(*m_layer, getDeviceId(1))
    .Times(1)
    .WillOnce(Return("DeviceId1"));
  EXPECT_CALL(*m_layer, isMainDisplay(1))
    .Times(1)
    .WillOnce(Return(true));

  EXPECT_TRUE(m_mac_dd.isPrimary("DeviceId1"));
}

TEST_F_S(SetAsPrimaryStub) {
  EXPECT_FALSE(m_mac_dd.setAsPrimary("DeviceId1"));
}

TEST_F_S(HdrStubs) {
  const auto states {m_mac_dd.getCurrentHdrStates({"DeviceId1", "DeviceId2"})};

  ASSERT_EQ(states.size(), 2U);
  EXPECT_EQ(states.at("DeviceId1"), std::nullopt);
  EXPECT_EQ(states.at("DeviceId2"), std::nullopt);

  EXPECT_TRUE(m_mac_dd.setHdrStates({}));
  EXPECT_TRUE(m_mac_dd.setHdrStates({{"DeviceId1", std::nullopt}}));
  EXPECT_FALSE(m_mac_dd.setHdrStates({{"DeviceId1", display_device::HdrState::Enabled}}));
}

TEST_F_S_SYSTEM(SetCurrentDisplayMode) {
  const auto active_displays {m_layer->getDisplayIds(display_device::MacQueryType::Active)};
  ASSERT_FALSE(active_displays.empty());

  std::string device_id;
  display_device::MacDisplayMode current_mode;
  display_device::MacDisplayMode alternate_mode;
  bool found_alternate {false};
  for (const auto display_id : active_displays) {
    const auto maybe_current_mode {m_layer->getCurrentDisplayMode(display_id)};
    const auto modes {m_layer->getDisplayModes(display_id)};
    if (!maybe_current_mode || modes.empty()) {
      continue;
    }

    const auto alternate_it {std::ranges::find_if(modes, [&maybe_current_mode](const auto &mode) {
      return !display_device::mac_utils::fuzzyCompareModes(mode, *maybe_current_mode);
    })};
    if (alternate_it == std::end(modes)) {
      continue;
    }

    device_id = m_layer->getDeviceId(display_id);
    current_mode = *maybe_current_mode;
    alternate_mode = *alternate_it;
    found_alternate = !device_id.empty();
    if (found_alternate) {
      break;
    }
  }

  if (!found_alternate) {
    GTEST_SKIP_("No active macOS display exposes an alternate desktop display mode.");
  }

  const display_device::MacDeviceDisplayModeMap original_modes {
    {device_id, current_mode}
  };
  const MacModeGuard mode_guard {m_mac_dd, original_modes};

  ASSERT_TRUE(m_mac_dd.setDisplayModes({{device_id, alternate_mode}}));
  const auto changed_modes {m_mac_dd.getCurrentDisplayModes({device_id})};
  ASSERT_EQ(changed_modes.size(), 1U);
  EXPECT_TRUE(display_device::mac_utils::fuzzyCompareModes(changed_modes.at(device_id), alternate_mode));
}
