CPMAddPackage (
    NAME StableCoder-cmake-scripts
    GITHUB_REPOSITORY StableCoder/cmake-scripts
    GIT_TAG 1f822d1fc87c8d7720c074cde8a278b44963c354
)

include(${StableCoder-cmake-scripts_SOURCE_DIR}/c++-standards.cmake)
include(${StableCoder-cmake-scripts_SOURCE_DIR}/compiler-options.cmake)

if(USE_SANITIZER)
    include(${StableCoder-cmake-scripts_SOURCE_DIR}/sanitizers.cmake)
endif()

if (USE_CCACHE)
    CPMAddPackage (
            NAME Ccache.cmake
            GITHUB_REPOSITORY TheLartians/Ccache.cmake
            VERSION 1.2.2
    )
endif()