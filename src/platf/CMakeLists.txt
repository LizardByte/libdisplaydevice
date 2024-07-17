# A global identifier for the library
set(MODULE libplatf)

# Globing headers (so that they appear in some IDEs) and sources
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "include/display_device/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

# Automatic library - will be static or dynamic based on user setting
add_library(${MODULE} ${HEADER_LIST} ${SOURCE_LIST})

# Provide the includes together with this library
target_include_directories(${MODULE} PUBLIC include)

# Shared libraries between platforms
target_link_libraries(${MODULE} PRIVATE libcommon)

# Link the platform specific library privately
if(WIN32)
    target_link_libraries(${MODULE} PRIVATE libwindows)
elseif(APPLE)
    message(WARNING "MacOS is not supported yet.")
elseif(UNIX)
    message(WARNING "Linux is not supported yet.")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
