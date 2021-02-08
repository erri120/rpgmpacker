#pragma once

#include <string>
#include <set>

#include <spdlog/spdlog.h>
#include <ghc/filesystem.hpp>
#include <simdjson.h>

struct ParsedData {
    //Actors.json
    //img/sv_actors/{}.png
    std::set<std::string> actorBattlerNames;

    //Animations.json
    //img/animations/{}.png
    std::set<std::string> animationNames;

    //Enemies.json
    //img/enemies/{}.png
    std::set<std::string> enemyBattlerNames;

    //Tilesets.json
    //img/tilesets/{}.png+{}.txt
    std::set<std::string> tilesetNames;

    //System.json
    //img/titles1/{}.png
    std::set<std::string> title1Names;
    //img/titles2/{}.png
    std::set<std::string> title2Names;

    //other
    //img/characters/{}.png
    std::set<std::string> characterNames;
    //img/faces/{}.png
    std::set<std::string> faceNames;

    //audio/bgm/{}.m4a/ogg
    std::set<std::string> bgmNames;
    //audio/bgs/{}.m4a/ogg
    std::set<std::string> bgsNames;
    //audio/me/{}.m4a/ogg
    std::set<std::string> meNames;
    //audio/se/{}.m4a/ogg
    std::set<std::string> seNames;

    //img/pictures/{}.png
    std::set<std::string> pictureNames;
    //movies/{}.webm
    std::set<std::string> movieNames;

    //img/battlebacks1/{}.png
    std::set<std::string> battleback1Names;
    //img/battlebacks2/{}.png
    std::set<std::string> battleback2Names;
    //img/parallaxes/{}.png
    std::set<std::string> parallaxNames;
};

bool parseData(const ghc::filesystem::path& dataFolder, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger);
bool parseEvents(simdjson::dom::array& eventList, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);

bool parseActors(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);
bool parseAnimations(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);
bool parseCommonEvents(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);
bool parseEnemies(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);
bool parseMap(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);
bool parseSystem(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);
bool parseTilesets(const ghc::filesystem::path& path, struct ParsedData* parsedData, const std::shared_ptr<spdlog::logger>& errorLogger);