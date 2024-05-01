#
# Setup google test
#
enable_testing()
set(INSTALL_GTEST OFF)
set(INSTALL_GMOCK OFF)
include(GoogleTest)
add_subdirectory("${PROJECT_SOURCE_DIR}/third-party/googletest" "third-party/googletest")

# A helper function to setup the test executables
function(add_dd_tests)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs TESTS LIBRARIES)
    cmake_parse_arguments(FN_VARS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(test ${FN_VARS_TESTS})
        add_executable(${test} ${test}.cpp)
        target_link_libraries(${test}
                gtest
                gtest_main  # if we use this we don't need our own main function
                libdisplaydevice # we are always testing at least the public API so it's safe to always link this
                libfixtures # these are our fixtures/helpers for the tests
                ${FN_VARS_LIBRARIES} # additional libraries if needed
        )

        # Adds the test to CTest
        gtest_discover_tests(${test})
    endforeach()
endfunction()

#
# Add subdirectories
#
add_subdirectory(fixtures)
add_subdirectory(unit)

#
# Additional setup for coverage
# https://gcovr.com/en/stable/guide/compiling.html#compiler-options
#
set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage -O1")
set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage -O1")

# TODO: why is this needed?
if (WIN32)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # cmake-lint: disable=C0103
endif ()
