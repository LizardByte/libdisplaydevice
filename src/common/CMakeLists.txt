# A global identifier for the library
set(MODULE libcommon)

# Globing headers here so that they appear in some IDEs
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "include/libdisplaydevice/*.h")

# Automatic library - will be static or dynamic based on user setting
add_library(${MODULE} ${HEADER_LIST} logging.cpp)

# Provide the includes together with this library
target_include_directories(${MODULE} PUBLIC include)
