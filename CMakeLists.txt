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

# if windows
if(WIN32)
    include(${CMAKE_MODULE_PATH}/windows.cmake)
elseif(APPLE)
    if(NOT CODE_QL_ANALYSIS)
        message(FATAL_ERROR "MacOS is not supported yet.")
    endif()
    # include(${CMAKE_MODULE_PATH}/macos.cmake)
else()
    if(NOT CODE_QL_ANALYSIS)
        message(FATAL_ERROR "Linux is not supported yet.")
    endif()
    # include(${CMAKE_MODULE_PATH}/linux.cmake)
endif()

# glob src
file(GLOB_RECURSE DD_TARGET_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
include_directories("${CMAKE_SOURCE_DIR}")

# tests
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# lib
add_library(${PROJECT_NAME} ${DD_TARGET_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
