# Add the test files in this directory
add_dd_test_dir(
        ADDITIONAL_LIBRARIES
        libwindows

        ADDITIONAL_SOURCES
        mocks/*.h
        mocks/*.cpp
)
