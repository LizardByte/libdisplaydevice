// header include
#include "fixtures.h"

// local includes
#include "displaydevice/logging.h"

BaseTest::BaseTest():
    m_sbuf { nullptr }, m_pipe_stdout { nullptr }, m_pipe_stderr { nullptr } {
  // intentionally empty
}

void
BaseTest::SetUp() {
  // todo: only run this one time, instead of every time a test is run
  // see: https://stackoverflow.com/questions/2435277/googletest-accessing-the-environment-from-a-test
  // get command line args from the test executable
  m_test_args = ::testing::internal::GetArgvs();

  // then get the directory of the test executable
  // std::string path = ::testing::internal::GetArgvs()[0];
  m_test_binary = m_test_args[0];

  // get the directory of the test executable
  m_test_binary_dir = std::filesystem::path(m_test_binary).parent_path();

  // If testBinaryDir is empty or `.` then set it to the current directory
  // maybe some better options here: https://stackoverflow.com/questions/875249/how-to-get-current-directory
  if (m_test_binary_dir.empty() || m_test_binary_dir.string() == ".") {
    m_test_binary_dir = std::filesystem::current_path();
  }

  m_sbuf = std::cout.rdbuf();  // save cout buffer (std::cout)
  std::cout.rdbuf(m_cout_buffer.rdbuf());  // redirect cout to buffer (std::cout)

  // Default to the verbose level in case some test fails
  display_device::Logger::get().setLogLevel(display_device::Logger::LogLevel::verbose);
}

void
BaseTest::TearDown() {
  display_device::Logger::get().setCustomCallback(nullptr);  // restore the default callback to avoid potential leaks
  std::cout.rdbuf(m_sbuf);  // restore cout buffer

  // get test info
  const ::testing::TestInfo *const test_info = ::testing::UnitTest::GetInstance()->current_test_info();

  if (test_info->result()->Failed()) {
    std::cout << std::endl
              << "Test failed: " << test_info->name() << std::endl
              << std::endl
              << "Captured cout:" << std::endl
              << m_cout_buffer.str() << std::endl
              << "Captured stdout:" << std::endl
              << m_stdout_buffer.str() << std::endl
              << "Captured stderr:" << std::endl
              << m_stderr_buffer.str() << std::endl;
  }

  m_sbuf = nullptr;  // clear sbuf
  if (m_pipe_stdout) {
    pclose(m_pipe_stdout);
    m_pipe_stdout = nullptr;
  }
  if (m_pipe_stderr) {
    pclose(m_pipe_stderr);
    m_pipe_stderr = nullptr;
  }
}

int
BaseTest::exec(const char *cmd) {
  std::array<char, 128> buffer {};
  m_pipe_stdout = popen((std::string(cmd) + " 2>&1").c_str(), "r");
  m_pipe_stderr = popen((std::string(cmd) + " 2>&1").c_str(), "r");
  if (!m_pipe_stdout || !m_pipe_stderr) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), m_pipe_stdout) != nullptr) {
    m_stdout_buffer << buffer.data();
  }
  while (fgets(buffer.data(), buffer.size(), m_pipe_stderr) != nullptr) {
    m_stderr_buffer << buffer.data();
  }
  int returnCode = pclose(m_pipe_stdout);
  m_pipe_stdout = nullptr;
  if (returnCode != 0) {
    std::cout << "Error: " << m_stderr_buffer.str() << std::endl
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
