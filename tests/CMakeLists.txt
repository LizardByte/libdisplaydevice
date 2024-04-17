cmake_minimum_required(VERSION 3.13)
# https://github.com/google/oss-policies-info/blob/main/foundational-cxx-support-matrix.md#foundational-c-support

project(test_libdisplaydevice)

include_directories("${CMAKE_SOURCE_DIR}")

enable_testing()

# Add GoogleTest directory to the project
set(GTEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third-party/googletest")
set(INSTALL_GTEST OFF)
set(INSTALL_GMOCK OFF)
add_subdirectory("${GTEST_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/googletest")
include_directories("${GTEST_SOURCE_DIR}/googletest/include" "${GTEST_SOURCE_DIR}")

# coverage
# https://gcovr.com/en/stable/guide/compiling.html#compiler-options
set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage -O1")
set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage -O1")

# if windows
if (WIN32)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # cmake-lint: disable=C0103
endif ()

set(TEST_PATTERNS test_* mock_*)
file(GLOB_RECURSE TEST_SOURCES
        "${CMAKE_SOURCE_DIR}/tests/conftest.cpp"
        "${CMAKE_SOURCE_DIR}/tests/utils.cpp")
set(TEST_PLATFORM_SOURCES)

foreach(pattern ${TEST_PATTERNS})
    file(GLOB_RECURSE CURRENT_TEST_SOURCES
            "${CMAKE_SOURCE_DIR}/tests/${pattern}.cpp")
    list(APPEND TEST_SOURCES ${CURRENT_TEST_SOURCES})

    if(WIN32)
        file(GLOB_RECURSE CURRENT_TEST_PLATFORM_SOURCES
                "${CMAKE_SOURCE_DIR}/tests/*/platf/windows/${pattern}.cpp")
    elseif(__APPLE__)
        file(GLOB_RECURSE CURRENT_TEST_PLATFORM_SOURCES
                "${CMAKE_SOURCE_DIR}/tests/*/platf/macos/${pattern}.cpp")
    elseif(UNIX)
        file(GLOB_RECURSE CURRENT_TEST_PLATFORM_SOURCES
                "${CMAKE_SOURCE_DIR}/tests/*/platf/linux/${pattern}.cpp")
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()

    list(APPEND TEST_PLATFORM_SOURCES ${CURRENT_TEST_PLATFORM_SOURCES})
endforeach()

list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/platf/.*")
message(STATUS "Common Test sources: ${TEST_SOURCES}")

set(DD_SOURCES
        ${DD_TARGET_FILES})

add_executable(${PROJECT_NAME}
        ${TEST_SOURCES}
        ${TEST_PLATFORM_SOURCES}
        ${DD_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME}
        ${DD_EXTERNAL_LIBRARIES}
        gtest
        gtest_main)  # if we use this we don't need our own main function
target_compile_definitions(${PROJECT_NAME} PUBLIC ${DD_DEFINITIONS} ${TEST_DEFINITIONS})
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${DD_COMPILE_OPTIONS}> -std=c++20)
target_link_options(${PROJECT_NAME} PRIVATE)

add_test(NAME ${PROJECT_NAME} COMMAND libdisplaydevice_test)