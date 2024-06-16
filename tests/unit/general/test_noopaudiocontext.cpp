// local includes
#include "displaydevice/noopaudiocontext.h"
#include "fixtures/fixtures.h"

namespace {
  // Test fixture(s) for this file
  class NoopAudioContextTest: public BaseTest {
  public:
    display_device::NoopAudioContext m_impl;
  };

  // Specialized TEST macro(s) for this test file
#define TEST_F_S(...) DD_MAKE_TEST(TEST_F, NoopAudioContextTest, __VA_ARGS__)
}  // namespace

TEST_F_S(Capture) {
  EXPECT_TRUE(m_impl.capture({}));
  EXPECT_TRUE(m_impl.capture({ "DeviceId1" }));
}

TEST_F_S(Release) {
  EXPECT_NO_THROW(m_impl.release({}));
  EXPECT_NO_THROW(m_impl.release({ "DeviceId1" }));
}
