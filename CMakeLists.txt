cmake_minimum_required(VERSION 3.13)  # todo: what is the minimum version required?

project(libdisplaydevice
        DESCRIPTION "Library to modify display devices."
        HOMEPAGE_URL "https://app.lizardbyte.dev")

set(PROJECT_LICENSE "GPL-3.0")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

# set the module path, used for includes
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# options
option(BUILD_TESTS "Build tests" ON)
option(CODE_QL_ANALYSIS "Build is for codeql analysis" OFF)

# auto-set CODE_QL_ANALYSIS based on env variable
if(DEFINED ENV{GITHUB_CODEQL_BUILD})
    set(CODE_QL_ANALYSIS ON)
endif()

# include directories (for including header files)
include_directories("${CMAKE_SOURCE_DIR}")

# glob src files, excluding platf directory
file(GLOB_RECURSE DD_COMMON_TARGET_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER DD_COMMON_TARGET_FILES EXCLUDE REGEX ".*/platf/.*")

if(WIN32)
    file(GLOB_RECURSE DD_PLATFORM_TARGET_FILES "${CMAKE_SOURCE_DIR}/src/platf/windows/*.cpp")
    include(${CMAKE_MODULE_PATH}/windows.cmake)
elseif(APPLE)
    if(NOT CODE_QL_ANALYSIS)
        message(FATAL_ERROR "MacOS is not supported yet.")
    endif()
    file(GLOB_RECURSE DD_PLATFORM_TARGET_FILES "${CMAKE_SOURCE_DIR}/src/platf/macos/*.cpp")
    include(${CMAKE_MODULE_PATH}/macos.cmake)
elseif(UNIX)
    if(NOT CODE_QL_ANALYSIS)
        message(FATAL_ERROR "Linux is not supported yet.")
    endif()
    file(GLOB_RECURSE DD_PLATFORM_TARGET_FILES "${CMAKE_SOURCE_DIR}/src/platf/linux/*.cpp")
    include(${CMAKE_MODULE_PATH}/linux.cmake)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

message(STATUS "Common source files: ${DD_COMMON_TARGET_FILES}")
message(STATUS "Platform source files: ${DD_PLATFORM_TARGET_FILES}")

# Combine common and platform-specific source files
set(DD_TARGET_FILES ${DD_COMMON_TARGET_FILES} ${DD_PLATFORM_TARGET_FILES})

# tests
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# lib
add_library(${PROJECT_NAME} ${DD_TARGET_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
