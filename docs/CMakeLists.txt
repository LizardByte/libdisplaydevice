# find doxygen and graphviz
find_package(Doxygen
        REQUIRED dot)

# define variables based on whether we are building on readthedocs
if(DEFINED ENV{READTHEDOCS})
    set(DOXYGEN_BUILD_DIR_CMAKE $ENV{READTHEDOCS_OUTPUT})
    set(DOXYGEN_PROJECT_VERSION $ENV{READTHEDOCS_VERSION})
else()
    set(DOXYGEN_BUILD_DIR_CMAKE "${CMAKE_CURRENT_BINARY_DIR}/build")
    set(DOXYGEN_PROJECT_VERSION ${PROJECT_VERSION})
endif()
message(STATUS "DOXYGEN_BUILD_DIR_CMAKE: ${DOXYGEN_BUILD_DIR_CMAKE}")

# download icon and logo
file(DOWNLOAD
        "https://raw.githubusercontent.com/LizardByte/.github/master/branding/logos/favicon.ico"
        "${DOXYGEN_BUILD_DIR_CMAKE}/lizardbyte.ico"
)
file(DOWNLOAD
        "https://raw.githubusercontent.com/LizardByte/.github/master/branding/logos/logo-128x128.png"
        "${DOXYGEN_BUILD_DIR_CMAKE}/lizardbyte.png"
)

# create build directories, as doxygen fails to create it in some cases?
file(MAKE_DIRECTORY "${DOXYGEN_BUILD_DIR_CMAKE}/html")

# convert to relative path, so doxygen doesn't get confused on Windows
file(RELATIVE_PATH DOXYGEN_BUILD_DIR_RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${DOXYGEN_BUILD_DIR_CMAKE}")
message(STATUS "DOXYGEN_BUILD_DIR_RELATIVE: ${DOXYGEN_BUILD_DIR_RELATIVE}")

# build docs
add_custom_target(docs ALL
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building Doxygen documentation"
        COMMAND ${CMAKE_COMMAND} -E env
            LDD_DOXYGEN_BUILD_DIR=${DOXYGEN_BUILD_DIR_RELATIVE}
            LDD_PROJECT_VERSION=${DOXYGEN_PROJECT_VERSION}
            ${DOXYGEN_EXECUTABLE} Doxyfile
        VERBATIM
)
