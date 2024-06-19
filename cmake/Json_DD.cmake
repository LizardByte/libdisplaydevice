#
# Loads the nlohmann_json library giving the priority to the system package first, with a fallback
# to the submodule.
#
find_package(nlohmann_json 3.11 QUIET)
if(NOT nlohmann_json_FOUND)
    include_guard(GLOBAL)
    message(STATUS "nlohmann_json v3.11.x package not found in the system. Falling back to submodule.")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../third-party/json third-party/json)
endif()
