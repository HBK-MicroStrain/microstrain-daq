cmake_minimum_required(VERSION 3.20)

include(cmake/git_version.cmake)
get_project_version_from_git_tag("v" REPO_FULL_VERSION REPO_NUMERIC_VERSION)

project(MicroStrainDaqUtils VERSION ${REPO_NUMERIC_VERSION})
set(PROJECT_VERSION ${REPO_FULL_VERSION})

find_package(openDAQ 3.40.0 EXACT QUIET)
if(NOT openDAQ_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        openDAQ
        GIT_REPOSITORY https://github.com/openDAQ/openDAQ.git
        GIT_TAG        v3.40.0
    )
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(OPENDAQ_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
    set(OPENDAQ_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(OPENDAQ_ENABLE_SIMULATOR OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(openDAQ)
endif()

add_library(microstrain_daq_utils
    src/daq_utils.cpp
    src/wireless.cpp
)
add_library(microstrain::daq_utils ALIAS microstrain_daq_utils)

target_include_directories(microstrain_daq_utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(microstrain_daq_utils PRIVATE daq::opendaq)
target_compile_features(microstrain_daq_utils PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(microstrain_daq_utils PRIVATE /MP)
endif()
