#include <string>
#include <vector>

#include <doctest/doctest.h>
#include <parseData.hpp>

#include "testingUtils.h"

#define FILE_EXISTS(file) {                                                             \
REQUIRE(!file.empty());                                                                 \
REQUIRE_MESSAGE(ghc::filesystem::is_regular_file(file), "File does not exist: ", file); \
}

#define CHECK_DATA(parsed, values) {                        \
CHECK(parsedData->parsed.size() == values.size());          \
for (const auto& s : parsedData->parsed) {                  \
    auto it = std::find(values.begin(), values.end(), s);   \
    CHECK_MESSAGE(it != values.end(), "Unable to find \"", s, "\" from parsed data vector ", #parsed, " in ", #values); \
}                                                           \
}

TEST_SUITE("Parsing data files") {
    TEST_CASE("Parse Actors.json") {
        SUBCASE("MV: Actors.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MV").append("data").append("Actors.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseActors(file, parsedData, RPGMakerVersion::MV, *loggers));

            static const std::vector<std::string> actorBattlerNames = {"Actor1_1", "Actor1_8", "Actor2_7", "Actor3_8"};
            static const std::vector<std::string> characterNames = {"Actor1", "Actor2", "Actor3"};
            static const std::vector<std::string> faceNames = {"Actor1", "Actor2", "Actor3"};

            CHECK_DATA(actorBattlerNames, actorBattlerNames)
            CHECK_DATA(characterNames, characterNames)
            CHECK_DATA(faceNames, faceNames)
        }

        SUBCASE("MZ: Actors.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MZ").append("data").append("Actors.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseActors(file, parsedData, RPGMakerVersion::MZ, *loggers));

            static const std::vector<std::string> actorBattlerNames = {"Actor1_1", "Actor1_2", "Actor1_3", "Actor1_4", "Actor1_5", "Actor1_6", "Actor1_7", "Actor1_8"};
            static const std::vector<std::string> characterNames = {"Actor1"};
            static const std::vector<std::string> faceNames = {"Actor1"};

            CHECK_DATA(actorBattlerNames, actorBattlerNames)
            CHECK_DATA(characterNames, characterNames)
            CHECK_DATA(faceNames, faceNames)
        }
    }
}