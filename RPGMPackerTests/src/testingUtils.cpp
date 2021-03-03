#include "testingUtils.h"

auto logger = spdlog::stdout_color_mt("console");
auto errorLogger = spdlog::stderr_color_mt("stderr");
const struct Loggers loggers(logger, errorLogger);

const struct Loggers* getLoggers() {
    return &loggers;
}

ghc::filesystem::path g_testFilesFolder = "";

ghc::filesystem::path getTestFilesFolder() {
    if (g_testFilesFolder.empty())
        g_testFilesFolder = ghc::filesystem::path(ghc::filesystem::current_path()).append("files");
    return g_testFilesFolder;
}

struct ParsedData* createParsedData() {
    return new ParsedData();
}