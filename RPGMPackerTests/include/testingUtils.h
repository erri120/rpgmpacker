#pragma once

#include <ghc/filesystem.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <loggers.h>
#include <parseData.hpp>

#define FILE_EXISTS(file) {                                                             \
REQUIRE(!file.empty());                                                                 \
REQUIRE_MESSAGE(ghc::filesystem::is_regular_file(file), "File does not exist: ", file); \
}

const struct Loggers* getLoggers();
ghc::filesystem::path getTestFilesFolder();
struct ParsedData* createParsedData();