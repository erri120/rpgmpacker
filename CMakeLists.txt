cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
    RPGMPacker
    LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
        "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

include(cmake/CPM.cmake)
include(cmake/Packages.cmake)
include(cmake/Tools.cmake)

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/**")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**")

add_executable(RPGMPacker ${headers} ${sources})

target_include_directories(
    RPGMPacker PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT-NAME}>
)

target_link_libraries(RPGMPacker cxxopts)
target_link_libraries(RPGMPacker spdlog)
target_link_libraries(RPGMPacker ghc_filesystem)
target_link_libraries(RPGMPacker Taskflow)
target_link_libraries(RPGMPacker simdjson)
target_link_libraries(RPGMPacker toml11)

#from StableCoder/cmake-scripts/c++-standards.cmake
cxx_17()

target_compile_options(RPGMPacker PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")
