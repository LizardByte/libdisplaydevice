#
# Loads the boost library giving the priority to the system package first, with a fallback
# to the submodule.
#
include_guard(GLOBAL)

# Limit boost to the required libraries only
set(REQUIRED_HEADER_LIBRARIES
        algorithm
        scope
        preprocessor
        uuid
)

find_package(Boost 1.87 CONFIG QUIET GLOBAL)
if(NOT Boost_FOUND)
    message(STATUS "Boost v1.87.x package not found in the system. Falling back to FetchContent.")
    include(FetchContent)

    set(BOOST_INCLUDE_LIBRARIES ${REQUIRED_HEADER_LIBRARIES})
    FetchContent_Declare(
            Boost
            URL      https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
            URL_HASH MD5=D55D43218E81CA3D0FC14436B7665BF1
            DOWNLOAD_EXTRACT_TIMESTAMP
    )
    FetchContent_MakeAvailable(Boost)
else()
    # For whatever reason the Boost::headers from find_package is not the same as the one from FetchContent
    # (differ in linked targets).
    # Also, FetchContent creates Boost::<lib> targets, whereas find_package does not. Since we cannot extend
    # Boost::headers as it is an ALIAS target, this is the workaround:
    get_target_property(original_headers_target Boost::headers ALIASED_TARGET)
    foreach (lib ${REQUIRED_HEADER_LIBRARIES})
        if (NOT TARGET Boost::${lib})
            add_library(Boost::${lib} ALIAS ${original_headers_target})
        endif ()
    endforeach ()
endif()
