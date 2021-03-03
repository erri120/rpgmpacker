cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
        RPGMPackerTests
        LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
            FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

add_executable(RPGMPackerTests src/main.cpp src/parsing.cpp include/testingUtils.h src/testingUtils.cpp)

target_include_directories(
        RPGMPackerTests PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT-NAME}>
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../RPGMPackerLib/)

target_link_libraries(RPGMPackerTests RPGMPackerLib)
target_link_libraries(RPGMPackerTests doctest::doctest)

cxx_17()

target_compile_options(RPGMPackerTests PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive>")

enable_testing()

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(RPGMPackerTests)

add_custom_command(TARGET RPGMPackerTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/files $<TARGET_FILE_DIR:RPGMPackerTests>/files)

if(ENABLE_TEST_COVERAGE)
    target_compile_options(RPGMPackerTests PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(RPGMPackerTests PUBLIC -fprofile-arcs -ftest-coverage)
endif()