#
# Loads the boost library giving the priority to the system package first, with a fallback
# to the submodule.
#
include_guard(GLOBAL)

find_package(Boost 1.85)
if(NOT Boost_FOUND)
    message(STATUS "Boost v1.85.x package not found in system. Falling back to Fetch.")
    include(FetchContent)

    # Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
        cmake_policy(SET CMP0135 NEW)
    endif()

    # Limit boost to the required libraries only
    set(BOOST_INCLUDE_LIBRARIES
            algorithm
            scope
            uuid)
    FetchContent_Declare(
            Boost
            URL      https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.tar.xz
            URL_HASH MD5=BADEA970931766604D4D5F8F4090B176
    )
    FetchContent_MakeAvailable(Boost)
endif()
