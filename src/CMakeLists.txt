# This a shared common library for other libraries
add_subdirectory(common)

# This is a platform-specific library
if(WIN32)
    add_subdirectory(windows)
elseif(APPLE)
    message(WARNING "MacOS is not supported yet.")
elseif(UNIX)
    message(WARNING "Linux is not supported yet.")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# This is a platform-specific library that loads the correct library for the OS
add_subdirectory(platf)

# This is the main library
add_subdirectory(displaydevice)
