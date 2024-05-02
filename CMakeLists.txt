#
# Project configuration
#
cmake_minimum_required(VERSION 3.13)  # todo: what is the minimum version required?
project(libdisplaydevice
        DESCRIPTION "Library to modify display devices."
        HOMEPAGE_URL "https://app.lizardbyte.dev"
        LANGUAGES CXX)

set(PROJECT_LICENSE "GPL-3.0")
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

#
# Project optional configuration
#
option(BUILD_TESTS "Build tests" ON)

#
# Library code is located here
#
add_subdirectory(src)

#
# Testing only available if this is the main project
#
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
