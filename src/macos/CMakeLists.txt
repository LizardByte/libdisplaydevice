# A global identifier for the library
set(MODULE libdisplaydevice_macos)
set(MODULE_ALIAS libdisplaydevice::platform)

# Globing headers (so that they appear in some IDEs) and sources
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "include/display_device/macos/*.h")
file(GLOB HEADER_DETAIL_LIST CONFIGURE_DEPENDS "include/display_device/macos/detail/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

# Automatic library - will be static or dynamic based on user setting
add_library(${MODULE} ${HEADER_LIST} ${HEADER_DETAIL_LIST} ${SOURCE_LIST})
add_library(${MODULE_ALIAS} ALIAS ${MODULE})

# Provide the includes together with this library
target_include_directories(${MODULE} PUBLIC include)

# Additional external libraries
include(Json_DD)

# Link the additional libraries
target_link_libraries(${MODULE} PRIVATE
        libdisplaydevice::common
        nlohmann_json::nlohmann_json
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework IOKit")
