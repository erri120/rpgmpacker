#pragma once

#include <spdlog/spdlog.h>
#include <ghc/filesystem.hpp>

#include "rpgmakerVersion.hpp"

typedef ghc::filesystem::path path;

struct InputPaths {
    //audio/bgm/
    path bgmPath;
    //audio/bgs/
    path bgsPath;
    //audio/me/
    path mePath;
    //audio/se/
    path sePath;

    //movies/
    path moviesPath;
    //img/pictures/
    path picturesPath;

    //img/titles1/
    path titles1Path;
    //img/titles2/
    path titles2Path;

    //img/characters/
    path charactersPath;
    //img/faces/
    path facesPath;

    //img/sv_actors/
    path actorsBattlerPath;
    //img/enemies/ (sv_enemies gets ignored or something)
    path enemiesBattlerPath;

    //MV only: img/animations/
    path animationsPath;
    //MZ only: effects/
    path effectsPath;

    //img/tilesets/
    path tilesetsPath;

    //img/battlebacks1/
    path battlebacks1Path;
    //img/battlebacks2/
    path battlebacks2Path;
    //img/parallaxes/
    path parallaxesPath;
};

bool getInputPaths(const path& inputFolder, struct InputPaths* inputPaths, RPGMakerVersion rpgMakerVersion, const std::shared_ptr<spdlog::logger>& errorLogger);
