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
    auto res = it == values.end();                          \
    CHECK_MESSAGE(!res, "Unable to find \"", s, "\" from parsed data vector ", #parsed, " in ", #values); \
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

    TEST_CASE("Parse Enemies.json") {
        SUBCASE("MV: Enemies.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MV").append("data").append("Enemies.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseEnemies(file, parsedData, RPGMakerVersion::MV, *loggers));

            static const std::vector<std::string> enemyBattlerNames = {"Bat", "Minotaur", "Orc", "Slime"};
            CHECK_DATA(enemyBattlerNames, enemyBattlerNames)
        }

        SUBCASE("MZ: Enemies.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MZ").append("data").append("Enemies.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseEnemies(file, parsedData, RPGMakerVersion::MZ, *loggers));

            static const std::vector<std::string> enemyBattlerNames = {"Crow", "Gnome", "Goblin", "Hi_monster", "Treant"};
            CHECK_DATA(enemyBattlerNames, enemyBattlerNames)
        }
    }

    TEST_CASE("Parse Tilesets.json") {
        static const std::vector<std::string> tilesetNames = {"Dungeon_A1", "Dungeon_A2", "Dungeon_A4",
                                                              "Dungeon_A5", "Dungeon_B",
                                                              "Dungeon_C", "Inside_A1", "Inside_A2", "Inside_A4",
                                                              "Inside_A5",
                                                              "Inside_B", "Inside_C", "Outside_A1", "Outside_A2",
                                                              "Outside_A3",
                                                              "Outside_A4", "Outside_A5", "Outside_B", "Outside_C",
                                                              "SF_Inside_A4",
                                                              "SF_Inside_B", "SF_Inside_C", "SF_Outside_A3",
                                                              "SF_Outside_A4", "SF_Outside_A5",
                                                              "SF_Outside_B", "SF_Outside_C", "World_A1",
                                                              "World_A2",
                                                              "World_B", "World_C"};

        SUBCASE("MV: Tilesets.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MV").append("data").append("Tilesets.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseTilesets(file, parsedData, RPGMakerVersion::MV, *loggers));
            CHECK_DATA(tilesetNames, tilesetNames)
        }

        SUBCASE("MZ: Tilesets.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MZ").append("data").append("Tilesets.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseTilesets(file, parsedData, RPGMakerVersion::MZ, *loggers));
            CHECK_DATA(tilesetNames, tilesetNames)
        }
    }

    TEST_CASE("Parse System.json") {
        SUBCASE("MV: System.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MV").append("data").append("System.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseSystem(file, parsedData, RPGMakerVersion::MV, *loggers));

            static const std::vector<std::string> enemyBattlerNames = {"Dragon"};
            static const std::vector<std::string> title1Names = {"Castle"};
            static const std::vector<std::string> title2Names = {"Medieval"};
            static const std::vector<std::string> bgmNames = {"Battle1", "Ship1", "Ship2", "Ship3", "Theme6"};
            static const std::vector<std::string> meNames = {"Defeat1", "Gameover1", "Victory1"};
            static const std::vector<std::string> seNames = {"Attack3", "Battle1", "Buzzer1", "Cancel2", "Collapse1",
                                                             "Collapse2", "Collapse3", "Collapse4", "Cursor2",
                                                             "Damage4", "Damage5", "Decision1", "Equip1", "Evasion1",
                                                             "Evasion2", "Item3", "Load", "Miss", "Recovery",
                                                             "Reflection", "Run", "Save", "Shop1"};
            static const std::vector<std::string> battleback1Names = {"Lava2"};
            static const std::vector<std::string> battleback2Names = {"DemonCastle3"};

            CHECK_DATA(enemyBattlerNames, enemyBattlerNames)
            CHECK_DATA(title1Names, title1Names)
            CHECK_DATA(title2Names, title2Names)
            CHECK_DATA(bgmNames, bgmNames)
            CHECK_DATA(meNames, meNames)
            CHECK_DATA(seNames, seNames)
            CHECK_DATA(battleback1Names, battleback1Names)
            CHECK_DATA(battleback2Names, battleback2Names)
        }

        SUBCASE("MZ: System.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MZ").append("data").append("System.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseSystem(file, parsedData, RPGMakerVersion::MZ, *loggers));

            static const std::vector<std::string> enemyBattlerNames = {"Hi_monster"};
            static const std::vector<std::string> title1Names = {"Ruins"};
            static const std::vector<std::string> title2Names = {"Medieval"};
            static const std::vector<std::string> bgmNames = {"Battle1", "Ship1", "Ship2", "Ship3", "Theme4"};
            static const std::vector<std::string> meNames = {"Defeat1", "Gameover1", "Victory1"};
            static const std::vector<std::string> seNames = {"Attack3", "Battle1", "Buzzer1", "Cancel2", "Collapse1",
                                                             "Collapse2", "Collapse3", "Collapse4", "Cursor3",
                                                             "Damage4", "Damage5", "Decision2", "Equip1", "Evasion1",
                                                             "Evasion2", "Item3", "Load2", "Miss", "Recovery",
                                                             "Reflection", "Run", "Save2", "Shop1"};
            static const std::vector<std::string> battleback1Names = {"GrassMaze"};
            static const std::vector<std::string> battleback2Names = {"GrassMaze"};

            CHECK_DATA(enemyBattlerNames, enemyBattlerNames)
            CHECK_DATA(title1Names, title1Names)
            CHECK_DATA(title2Names, title2Names)
            CHECK_DATA(bgmNames, bgmNames)
            CHECK_DATA(meNames, meNames)
            CHECK_DATA(seNames, seNames)
            CHECK_DATA(battleback1Names, battleback1Names)
            CHECK_DATA(battleback2Names, battleback2Names)
        }
    }

    TEST_CASE("Parse Map.json") {
        SUBCASE("MV: Map.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MV").append("data").append("Map001.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseMap(file, parsedData, RPGMakerVersion::MV, *loggers));

            static const std::vector<std::string> characterNames = {"Vehicle"};
            static const std::vector<std::string> faceNames = {"Evil"};
            static const std::vector<std::string> bgmNames = {"Town1"};
            static const std::vector<std::string> bgsNames = {"River"};
            static const std::vector<std::string> battleback1Names = {"Clouds"};
            static const std::vector<std::string> battleback2Names = {"Bridge"};
            static const std::vector<std::string> parallaxNames = {"Sunset"};

            CHECK_DATA(characterNames, characterNames)
            CHECK_DATA(faceNames, faceNames)
            CHECK_DATA(bgmNames, bgmNames)
            CHECK_DATA(bgsNames, bgsNames)
            CHECK_DATA(battleback1Names, battleback1Names)
            CHECK_DATA(battleback2Names, battleback2Names)
            CHECK_DATA(parallaxNames, parallaxNames)
        }

        SUBCASE("MZ: Map.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MZ").append("data").append("Map001.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseMap(file, parsedData, RPGMakerVersion::MZ, *loggers));

            static const std::vector<std::string> characterNames = {"Nature"};
            static const std::vector<std::string> faceNames = {"People4"};
            static const std::vector<std::string> bgmNames = {"Field1"};
            static const std::vector<std::string> bgsNames = {"City"};
            static const std::vector<std::string> battleback1Names = {"Castle1"};
            static const std::vector<std::string> battleback2Names = {"Brick"};
            static const std::vector<std::string> parallaxNames = {"BlueSky"};

            CHECK_DATA(characterNames, characterNames)
            CHECK_DATA(faceNames, faceNames)
            CHECK_DATA(bgmNames, bgmNames)
            CHECK_DATA(bgsNames, bgsNames)
            CHECK_DATA(battleback1Names, battleback1Names)
            CHECK_DATA(battleback2Names, battleback2Names)
            CHECK_DATA(parallaxNames, parallaxNames)
        }
    }

    TEST_CASE("Parse CommonEvents.json") {
        SUBCASE("MV: CommonEvents.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MV").append("data").append("CommonEvents.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseCommonEvents(file, parsedData, RPGMakerVersion::MV, *loggers));

            static const std::vector<std::string> actorBattlerNames = {"Actor1_2"};
            static const std::vector<std::string> characterNames = {"Actor1", "People2"};
            static const std::vector<std::string> faceNames = {"Actor1", "Vehicle"};
            static const std::vector<std::string> bgmNames = {"Battle1", "Battle3", "Ship2"};
            static const std::vector<std::string> bgsNames = {"Darkness"};
            static const std::vector<std::string> meNames = {"Defeat1", "Fanfare2", "Musical2"};
            static const std::vector<std::string> seNames = {"Battle1", "Battle2"};
            static const std::vector<std::string> pictureNames = {"nice"};
            static const std::vector<std::string> movieNames = {"dorime-output-web"};
            static const std::vector<std::string> battleback1Names = {"Clouds", "Crystal"};
            static const std::vector<std::string> battleback2Names = {"", "Fort1"};
            static const std::vector<std::string> parallaxNames = {"Mountains5"};

            CHECK_DATA(actorBattlerNames, actorBattlerNames)
            CHECK_DATA(characterNames, characterNames)
            CHECK_DATA(faceNames, faceNames)
            CHECK_DATA(bgmNames, bgmNames)
            CHECK_DATA(bgsNames, bgsNames)
            CHECK_DATA(meNames, meNames)
            CHECK_DATA(seNames, seNames)
            CHECK_DATA(pictureNames, pictureNames)
            CHECK_DATA(movieNames, movieNames)
            CHECK_DATA(battleback1Names, battleback1Names)
            CHECK_DATA(battleback2Names, battleback2Names)
            CHECK_DATA(parallaxNames, parallaxNames)
        }

        SUBCASE("MZ: CommonEvents.json") {
            auto file = ghc::filesystem::path(getTestFilesFolder()).append("MZ").append("data").append("CommonEvents.json");
            FILE_EXISTS(file)

            auto loggers = getLoggers();
            auto parsedData = createParsedData();

            CHECK(parseCommonEvents(file, parsedData, RPGMakerVersion::MZ, *loggers));

            static const std::vector<std::string> actorBattlerNames = {"Actor2_5"};
            static const std::vector<std::string> characterNames = {"Actor1", "Evil"};
            static const std::vector<std::string> faceNames = {"Actor1", "People1", "Vehicle"};
            static const std::vector<std::string> bgmNames = {"Battle7", "Castle2", "Theme3"};
            static const std::vector<std::string> bgsNames = {"Rain1"};
            static const std::vector<std::string> meNames = {"Inn2", "Organ"};
            static const std::vector<std::string> seNames = {"Autodoor", "Bell1"};
            static const std::vector<std::string> pictureNames = {"Actor3_6"};
            static const std::vector<std::string> movieNames = {""};
            static const std::vector<std::string> battleback1Names = {"DecorativeTile1"};
            static const std::vector<std::string> battleback2Names = {"Clouds"};
            static const std::vector<std::string> parallaxNames = {"River"};

            CHECK_DATA(actorBattlerNames, actorBattlerNames)
            CHECK_DATA(characterNames, characterNames)
            CHECK_DATA(faceNames, faceNames)
            CHECK_DATA(bgmNames, bgmNames)
            CHECK_DATA(bgsNames, bgsNames)
            CHECK_DATA(meNames, meNames)
            CHECK_DATA(seNames, seNames)
            CHECK_DATA(pictureNames, pictureNames)
            CHECK_DATA(movieNames, movieNames)
            CHECK_DATA(battleback1Names, battleback1Names)
            CHECK_DATA(battleback2Names, battleback2Names)
            CHECK_DATA(parallaxNames, parallaxNames)
        }
    }
}