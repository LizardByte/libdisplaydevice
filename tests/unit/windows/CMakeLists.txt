# Add the test files in this directory
add_dd_test_dir(
        ADDITIONAL_LIBRARIES
        Boost::scope
        libwindows

        ADDITIONAL_SOURCES
        utils/*.h
        utils/*.cpp
)
