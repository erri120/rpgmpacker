#pragma once

#include <string>
#include <set>

#include <spdlog/spdlog.h>
#include <ghc/filesystem.hpp>
#include <simdjson.h>

#include "rpgmakerVersion.hpp"

struct ParsedData {
    //Actors.json
    //img/sv_actors/{}.png
    std::set<std::string> actorBattlerNames;

    //Animations.json
    std::set<uint64_t> animationIds;
    //MV only: img/animations/{}.png
    std::set<std::string> animationNames;
    //MZ only: effects/{}.efkefc
    std::set<std::string> effectNames;
    //MZ only: effects/{}
    std::set<std::string> effectResources;

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

bool parseData(const ghc::filesystem::path& dataFolder, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseEvents(simdjson::dom::array& eventList, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);

bool parseActors(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseAnimations(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseCommonEvents(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseEnemies(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseItems(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseMap(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseSkills(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseSystem(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseTilesets(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseTroops(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);
bool parseWeapons(const ghc::filesystem::path& path, struct ParsedData* parsedData, RPGMakerVersion rpgMakerVersion, const struct Loggers& loggers);

struct EffectInfoChunk {
    uint32_t name;
    uint32_t size;
    uint32_t unknown;
};

struct EffectHeader {
    uint32_t magic;
    uint32_t version;
};

bool parseEffect(const ghc::filesystem::path& path, const ghc::filesystem::path& effectsPath, struct ParsedData* parsedData, const struct Loggers& loggers);