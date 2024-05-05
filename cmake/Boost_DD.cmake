#
# Loads the boost library giving the priority to the system package first, with a fallback
# to the submodule.
#
include_guard(GLOBAL)

find_package(Boost 1.85)
if(NOT Boost_FOUND)
    message(STATUS "Boost v1.85.x package not found in system. Falling back to submodule.")

    # Limit boost to the required libraries only
    set(BOOST_INCLUDE_LIBRARIES
            algorithm
            scope
            uuid)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../third-party/boost third-party/boost EXCLUDE_FROM_ALL)

    # Emulate the "find_package" target
    add_library(Boost::boost INTERFACE IMPORTED)

    file(GLOB BOOST_LIBS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../third-party/boost/libs/*")
    foreach(lib_path ${BOOST_LIBS})
        get_filename_component(lib_dir "${lib_path}" NAME)
        if(TARGET "Boost::${lib_dir}")
            target_link_libraries(Boost::boost INTERFACE "Boost::${lib_dir}")
        endif()
    endforeach()
endif()
