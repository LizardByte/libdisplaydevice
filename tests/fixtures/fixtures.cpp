// header include
#include "fixtures.h"

// local includes
#include "displaydevice/logging.h"

BaseTest::BaseTest():
    sbuf { nullptr }, pipe_stdout { nullptr }, pipe_stderr { nullptr } {
  // intentionally empty
}

void
BaseTest::SetUp() {
  // todo: only run this one time, instead of every time a test is run
  // see: https://stackoverflow.com/questions/2435277/googletest-accessing-the-environment-from-a-test
  // get command line args from the test executable
  testArgs = ::testing::internal::GetArgvs();

  // then get the directory of the test executable
  // std::string path = ::testing::internal::GetArgvs()[0];
  testBinary = testArgs[0];

  // get the directory of the test executable
  testBinaryDir = std::filesystem::path(testBinary).parent_path();

  // If testBinaryDir is empty or `.` then set it to the current directory
  // maybe some better options here: https://stackoverflow.com/questions/875249/how-to-get-current-directory
  if (testBinaryDir.empty() || testBinaryDir.string() == ".") {
    testBinaryDir = std::filesystem::current_path();
  }

  sbuf = std::cout.rdbuf();  // save cout buffer (std::cout)
  std::cout.rdbuf(cout_buffer.rdbuf());  // redirect cout to buffer (std::cout)

  // Default to the verbose level in case some test fails
  display_device::logger_t::get().set_log_level(display_device::logger_t::log_level_e::verbose);
}

void
BaseTest::TearDown() {
  display_device::logger_t::get().set_custom_callback(nullptr);  // restore the default callback to avoid potential leaks
  std::cout.rdbuf(sbuf);  // restore cout buffer

  // get test info
  const ::testing::TestInfo *const test_info = ::testing::UnitTest::GetInstance()->current_test_info();

  if (test_info->result()->Failed()) {
    std::cout << std::endl
              << "Test failed: " << test_info->name() << std::endl
              << std::endl
              << "Captured cout:" << std::endl
              << cout_buffer.str() << std::endl
              << "Captured stdout:" << std::endl
              << stdout_buffer.str() << std::endl
              << "Captured stderr:" << std::endl
              << stderr_buffer.str() << std::endl;
  }

  sbuf = nullptr;  // clear sbuf
  if (pipe_stdout) {
    pclose(pipe_stdout);
    pipe_stdout = nullptr;
  }
  if (pipe_stderr) {
    pclose(pipe_stderr);
    pipe_stderr = nullptr;
  }
}

int
BaseTest::exec(const char *cmd) {
  std::array<char, 128> buffer {};
  pipe_stdout = popen((std::string(cmd) + " 2>&1").c_str(), "r");
  pipe_stderr = popen((std::string(cmd) + " 2>&1").c_str(), "r");
  if (!pipe_stdout || !pipe_stderr) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe_stdout) != nullptr) {
    stdout_buffer << buffer.data();
  }
  while (fgets(buffer.data(), buffer.size(), pipe_stderr) != nullptr) {
    stderr_buffer << buffer.data();
  }
  int returnCode = pclose(pipe_stdout);
  pipe_stdout = nullptr;
  if (returnCode != 0) {
    std::cout << "Error: " << stderr_buffer.str() << std::endl
              << "Return code: " << returnCode << std::endl;
  }
  return returnCode;
}

void
LinuxTest::SetUp() {
#ifndef __linux__
  GTEST_SKIP_("Skipping, this test is for Linux only.");
#endif
  BaseTest::SetUp();
}

void
LinuxTest::TearDown() {
#ifndef __linux__
  // This a noop case to skip the teardown
  return;
#endif
  BaseTest::TearDown();
}

void
MacOSTest::SetUp() {
#if !defined(__APPLE__) || !defined(__MACH__)
  GTEST_SKIP_("Skipping, this test is for macOS only.");
#endif
  BaseTest::SetUp();
}

void
MacOSTest::TearDown() {
#if !defined(__APPLE__) || !defined(__MACH__)
  // This a noop case to skip the teardown
  return;
#endif
  BaseTest::TearDown();
}

void
WindowsTest::SetUp() {
#ifndef _WIN32
  GTEST_SKIP_("Skipping, this test is for Windows only.");
#endif
  BaseTest::SetUp();
}

void
WindowsTest::TearDown() {
#ifndef _WIN32
  // This a noop case to skip the teardown
  return;
#endif
  BaseTest::TearDown();
}
