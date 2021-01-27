cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(RPGMPacker LANGUAGES CXX)

include(cmake/CPM.cmake)

add_executable(RPGMPacker src/main.cpp)

set_target_properties(RPGMPacker PROPERTIES CXX_STANDARD 17)

target_compile_options(RPGMPacker PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")