#include <string>
#include <iterator>
#include <algorithm>

#include <doctest/doctest.h>
#include <parseData.hpp>

#include "testingUtils.h"

#define FILE_EXISTS(file) {                     \
CHECK(!file.empty());                           \
CHECK(ghc::filesystem::is_regular_file(file));  \
}

#define CHECK_DATA(vector, values) {            \
for (const auto& s : parsedData->vector) {      \
    auto res = false;                           \
    for (const auto& b : values) {              \
        if (s == b) {                           \
            res = true;                         \
            break;                              \
        }                                       \
    }                                           \
    CHECK(res);                                 \
}                                               \
}

TEST_SUITE("Parsing data files") {
    TEST_CASE("Parse Actors.json") {
        SUBCASE("MV: Actors.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MV").append("data").append("Actors.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseActors(file, parsedData, RPGMakerVersion::MV, *loggers));

            CHECK(parsedData->actorBattlerNames.size() == 4);
            CHECK(parsedData->characterNames.size() == 3);
            CHECK(parsedData->faceNames.size() == 3);

            static const std::string actorBattlerNames[] = {"Actor1_1", "Actor1_8", "Actor2_7", "Actor3_8"};
            static const std::string characterNames[] = {"Actor1", "Actor2", "Actor3"};
            static const std::string faceNames[] = {"Actor1", "Actor2", "Actor3"};

            CHECK_DATA(actorBattlerNames, actorBattlerNames)
            CHECK_DATA(characterNames, characterNames)
            CHECK_DATA(faceNames, faceNames)
        }
    }
}