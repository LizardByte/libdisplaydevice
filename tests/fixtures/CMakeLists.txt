# A global identifier for the library
set(MODULE libfixtures)

# Globing headers (so that they appear in some IDEs) and sources
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

# Automatic library - will be static or dynamic based on user setting
add_library(${MODULE} ${HEADER_LIST} ${SOURCE_LIST})

# Provide the includes together with this library
target_include_directories(${MODULE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link the additional libraries
target_link_libraries(${MODULE} PRIVATE gtest libcommon)
