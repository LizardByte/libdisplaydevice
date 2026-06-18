// system includes
#include <stdexcept>

// local includes
#include "display_device/macos/mac_display_device.h"
#include "fixtures/fixtures.h"
#include "utils/mock_mac_api_layer.h"

namespace {
  // Convenience keywords for GMock
  using ::testing::HasSubstr;
  using ::testing::Return;
  using ::testing::StrictMock;

  // Test fixture(s) for this file
  class MacDisplayDeviceMocked: public BaseTest {
  public:
    std::shared_ptr<StrictMock<display_device::MockMacApiLayer>> m_layer {std::make_shared<StrictMock<display_device::MockMacApiLayer>>()};
    display_device::MacDisplayDevice m_mac_dd {m_layer};
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, MacDisplayDeviceMocked, __VA_ARGS__)
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

TEST_F_S(GeneralStubs) {
  EXPECT_TRUE(m_mac_dd.enumAvailableDevices().empty());
  EXPECT_TRUE(m_mac_dd.getDisplayName("DeviceId1").empty());
}

TEST_F_S(TopologyStubs) {
  EXPECT_TRUE(m_mac_dd.getCurrentTopology().empty());
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

TEST_F_S(DisplayModeStubs) {
  EXPECT_TRUE(m_mac_dd.getCurrentDisplayModes({"DeviceId1"}).empty());
  EXPECT_FALSE(m_mac_dd.setDisplayModes({{"DeviceId1", {}}}));
}

TEST_F_S(PrimaryStubs) {
  EXPECT_FALSE(m_mac_dd.isPrimary("DeviceId1"));
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
