cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(RPGMPacker LANGUAGES CXX)

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

add_executable(RPGMPacker src/main.cpp)

target_link_libraries(RPGMPacker cxxopts)
target_link_libraries(RPGMPacker spdlog)
target_link_libraries(RPGMPacker ghc_filesystem)

set_target_properties(RPGMPacker PROPERTIES CXX_STANDARD 17)

target_compile_options(RPGMPacker PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")