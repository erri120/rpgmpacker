cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Umbrella)

include(cmake/CPM.cmake)
include(cmake/Tools.cmake)
include(cmake/Packages.cmake)

add_subdirectory(RPGMPacker)
add_subdirectory(RPGMPackerLib)
add_subdirectory(RPGMPackerTests)