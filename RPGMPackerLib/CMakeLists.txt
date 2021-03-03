cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(
        RPGMPackerLib
        LANGUAGES CXX
)

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
            FATAL_ERROR
            "In-source builds are not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif ()

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/**")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**")

add_library(RPGMPackerLib ${headers} ${sources})

target_include_directories(
        RPGMPackerLib PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT-NAME}>
)

#target_link_libraries(RPGMPackerLib cxxopts)
target_link_libraries(RPGMPackerLib spdlog)
target_link_libraries(RPGMPackerLib ghc_filesystem)
target_link_libraries(RPGMPackerLib Taskflow)
target_link_libraries(RPGMPackerLib simdjson)
target_link_libraries(RPGMPackerLib toml11)

cxx_17()

target_compile_options(RPGMPackerLib PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive>")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU"
        OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    # GCC/Clang
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
endif()