cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
    RPGMPacker
    LANGUAGES CXX
)

include(cmake/CPM.cmake)

CPMAddPackage (
    NAME cxxopts
    GITHUB_REPOSITORY jarro2783/cxxopts
    VERSION 2.2.1
    OPTIONS
        "CXXOPTS_BUILD_EXAMPLES Off"
        "CXXOPTS_BUILD_TESTS Off"
)

CPMAddPackage (
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.8.2
)

CPMAddPackage (
    NAME ghc_filesystem
    GITHUB_REPOSITORY gulrak/filesystem
    VERSION 1.4.0
    OPTIONS
        "GHC_FILESYSTEM_BUILD_TESTING Off"
        "GHC_FILESYSTEM_BUILD_EXAMPLES Off"
        "GHC_FILESYSTEM_WITH_INSTALL Off"
)

CPMAddPackage (
    NAME Taskflow
    GITHUB_REPOSITORY taskflow/taskflow
    VERSION 3.0.0
    OPTIONS
        "TF_BUILD_BENCHMARKS Off"
        "TF_BUILD_CUDA Off"
        "TF_BUILD_SYCL Off"
        "TF_BUILD_TESTS Off"
        "TF_BUILD_EXAMPLES Off"
        "TF_BUILD_PROFILER Off"
)


file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**")
add_executable(RPGMPacker ${sources})

target_link_libraries(RPGMPacker cxxopts)
target_link_libraries(RPGMPacker spdlog)
target_link_libraries(RPGMPacker ghc_filesystem)
target_link_libraries(RPGMPacker Taskflow)

set_target_properties(RPGMPacker PROPERTIES CXX_STANDARD 17)

target_compile_options(RPGMPacker PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")
