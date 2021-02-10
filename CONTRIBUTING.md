# Contributing

## Requirements

- [CMake](https://cmake.org/) (see `cmake_minimum_required` in [`CMakeLists.txt`](CMakeLists.txt))
- Compiler that supports C++17 (tested with MSVC 19.28.29336.0, MinGW 8.1.0, GNU 9.3.0 and AppleClang 12.0.0.12000032)
- IDE (recommend VS Code or CLion)

## Building from Source

```bash
#create build directory
cmake -E make_directory build

#configure CMake to build Debug to build/
cmake -S . -B build/ -DCMAKE_BUILD_TYPE=Debug

#build the project
cmake --build build/ --config Debug --target RPGMPacker
```

Dependencies are handled by [CPM](https://github.com/cpm-cmake/CPM.cmake) and I recommend using the [`CPM_SOURCE_CACHE`](https://github.com/cpm-cmake/CPM.cmake#cpm_source_cache) environment variable by adding `-DCPM_SOURCE_CACHE=<path to an external download directory>` to CMake so it doesn't re-download the same files. 

The build process ~~can~~ should be automated by an IDE like VS Code (use the recommended [`extensions.json`](.vscode/extensions.json)) or CLion.

On Windows make sure you build using Visual Studio x64: `-G "Visual Studio 16 2019" -A x64`.