CPMAddPackage ("gh:jarro2783/cxxopts@2.2.1")
CPMAddPackage ("gh:gabime/spdlog@1.8.5")
CPMAddPackage ("gh:taskflow/taskflow@3.0.0")
CPMAddPackage ("gh:ToruNiina/toml11@3.6.1")

CPMAddPackage (
        NAME ghc_filesystem
        GITHUB_REPOSITORY gulrak/filesystem
        VERSION 1.5.4
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

CPMAddPackage("gh:onqtam/doctest#2.4.5")
