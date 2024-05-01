# A global identifier for the library
set(MODULE libfixtures)

# Globing headers here so that they appear in some IDEs
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "*.h")

# Automatic library - will be static or dynamic based on user setting
add_library(${MODULE} ${HEADER_LIST} utils.cpp fixtures.cpp)

# Provide the includes together with this library
target_include_directories(${MODULE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the additional libraries
target_link_libraries(${MODULE} PRIVATE gtest libcommon)
