cmake_minimum_required(VERSION 3.13)  # todo: what is the minimum version required?

project(libdisplaydevice
        DESCRIPTION "Library to modify display devices."
        HOMEPAGE_URL "https://app.lizardbyte.dev")

set(PROJECT_LICENSE "GPL-3.0")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

# set the module path, used for includes
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# options
option(BUILD_TESTS "Build tests" ON)

# if windows
if(WIN32)
    include(${CMAKE_MODULE_PATH}/windows.cmake)
elseif(APPLE)
    message(FATAL_ERROR "MacOS is not supported yet.")
    # include(${CMAKE_MODULE_PATH}/macos.cmake)
else()
    message(FATAL_ERROR "Linux is not supported yet.")
    # include(${CMAKE_MODULE_PATH}/linux.cmake)
endif()

# glob src
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# tests
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# lib
add_library(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
