// local includes
#include "fixtures/fixtures.h"

TEST(HelloWorldTest, HelloWorld) {
  EXPECT_TRUE(true);
}

TEST_F(LinuxTest, LinuxTest) {
  EXPECT_TRUE(true);
}

TEST_F(MacOSTest, MacTest) {
  EXPECT_TRUE(true);
}

TEST_F(WindowsTest, WindowsTest) {
  EXPECT_TRUE(true);
}
