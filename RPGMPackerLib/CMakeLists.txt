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

#CPMAddPackage ("gh:jarro2783/cxxopts@2.2.1")
CPMAddPackage ("gh:gabime/spdlog@1.8.2")
CPMAddPackage ("gh:taskflow/taskflow@3.0.0")
CPMAddPackage ("gh:ToruNiina/toml11@3.6.0")

CPMAddPackage (
        NAME ghc_filesystem
        GITHUB_REPOSITORY gulrak/filesystem
        VERSION 1.5.2
        EXCLUDE_FROM_ALL YES
        OPTIONS
        "GHC_FILESYSTEM_ENFORCE_CPP17_API On"
)

CPMAddPackage (
        NAME simdjson
        GITHUB_REPOSITORY simdjson/simdjson
        VERSION 0.8.1
        OPTIONS
        "SIMDJSON_JUST_LIBRARY On"
        "SIMDJSON_BUILD_STATIC On"
)


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