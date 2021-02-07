cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
    RPGMPacker
    LANGUAGES CXX
)

include(cmake/CPM.cmake)
include(cmake/Packages.cmake)
include(cmake/Tools.cmake)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**")
add_executable(RPGMPacker ${sources})

target_link_libraries(RPGMPacker cxxopts)
target_link_libraries(RPGMPacker spdlog)
target_link_libraries(RPGMPacker ghc_filesystem)
target_link_libraries(RPGMPacker Taskflow)
target_link_libraries(RPGMPacker simdjson)

set_target_properties(RPGMPacker PROPERTIES CXX_STANDARD 17)

target_compile_options(RPGMPacker PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")
