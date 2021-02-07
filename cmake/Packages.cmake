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

CPMAddPackage (
        NAME simdjson
        GITHUB_REPOSITORY simdjson/simdjson
        VERSION 0.8.1
        OPTIONS
        "SIMDJSON_JUST_LIBRARY On"
        "SIMDJSON_BUILD_STATIC On"
)