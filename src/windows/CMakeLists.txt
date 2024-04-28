# A global identifier for the library
set(MODULE libwindows)

# Globing headers here so that they appear in some IDEs
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "include/libdisplaydevice/windows/*.h")

# Automatic library - will be static or dynamic based on user setting
add_library(${MODULE} ${HEADER_LIST} winapilayer.cpp)

# Provide the includes together with this library
target_include_directories(${MODULE} PUBLIC include)

# Link the additional libraries
target_link_libraries(${MODULE} PRIVATE libcommon)
