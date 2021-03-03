#pragma once

#include <ghc/filesystem.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <loggers.h>
#include <parseData.hpp>

const struct Loggers* getLoggers();
ghc::filesystem::path getTestFilesFolder();
struct ParsedData* createParsedData();