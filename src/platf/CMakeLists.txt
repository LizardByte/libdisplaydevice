# A global identifier for the library
set(MODULE libplatf)

# Globing headers here so that they appear in some IDEs
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "include/libdisplaydevice/*.h")

# Automatic library - will be static or dynamic based on user setting
add_library(${MODULE} ${HEADER_LIST} libplatfplaceholder.cpp)

# Provide the includes together with this library
target_include_directories(${MODULE} PUBLIC include)

# Shared libraries between platforms
target_link_libraries(${MODULE} PRIVATE libcommon)

# Link the platform specific library privately
if(WIN32)
    target_link_libraries(${MODULE} PRIVATE libwindows)
    target_compile_definitions(${MODULE} PUBLIC DD_WIN)
elseif(APPLE)
    message(WARNING "MacOS is not supported yet.")
    target_compile_definitions(${MODULE} PUBLIC DD_APPLE)
elseif(UNIX)
    message(WARNING "Linux is not supported yet.")
    target_compile_definitions(${MODULE} PUBLIC DD_UNIX)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
