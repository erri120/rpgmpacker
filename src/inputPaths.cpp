#include "inputPaths.hpp"

#include <spdlog/spdlog.h>
#include <ghc/filesystem.hpp>

#include "formatters.hpp"

#define FOLDER_EXISTS(folder) if (!ghc::filesystem::is_directory(folder, ec)) { \
errorLogger->warn("Folder does not exist: {}! {}", folder, ec); }

bool getInputPaths(const path& inputFolder, struct InputPaths* inputPaths, RPGMakerVersion rpgMakerVersion, const std::shared_ptr<spdlog::logger>& errorLogger) {
    std::error_code ec;

    auto audioFolder = path(inputFolder).append("audio");
    auto imgFolder = path(inputFolder).append("img");
    FOLDER_EXISTS(audioFolder)
    FOLDER_EXISTS(imgFolder)

    inputPaths->bgmPath = path(audioFolder).append("bgm");
    inputPaths->bgsPath = path(audioFolder).append("bgs");
    inputPaths->mePath = path(audioFolder).append("me");
    inputPaths->sePath = path(audioFolder).append("se");
    FOLDER_EXISTS(inputPaths->bgmPath)
    FOLDER_EXISTS(inputPaths->bgsPath)
    FOLDER_EXISTS(inputPaths->mePath)
    FOLDER_EXISTS(inputPaths->sePath)

    inputPaths->moviesPath = path(inputFolder).append("movies");
    inputPaths->picturesPath = path(imgFolder).append("pictures");
    FOLDER_EXISTS(inputPaths->moviesPath)
    FOLDER_EXISTS(inputPaths->picturesPath)

    inputPaths->titles1Path = path(imgFolder).append("titles1");
    inputPaths->titles2Path = path(imgFolder).append("titles2");
    FOLDER_EXISTS(inputPaths->titles1Path)
    FOLDER_EXISTS(inputPaths->titles2Path)

    inputPaths->charactersPath = path(imgFolder).append("characters");
    inputPaths->facesPath = path(imgFolder).append("faces");
    FOLDER_EXISTS(inputPaths->charactersPath)
    FOLDER_EXISTS(inputPaths->facesPath)

    inputPaths->actorsBattlerPath = path(imgFolder).append("sv_actors");
    inputPaths->enemiesBattlerPath = path(imgFolder).append("enemies");
    FOLDER_EXISTS(inputPaths->actorsBattlerPath)
    FOLDER_EXISTS(inputPaths->enemiesBattlerPath)

    inputPaths->tilesetsPath = path(imgFolder).append("tilesets");
    FOLDER_EXISTS(inputPaths->tilesetsPath)

    inputPaths->battlebacks1Path = path(imgFolder).append("battlebacks1");
    inputPaths->battlebacks2Path = path(imgFolder).append("battlebacks2");
    inputPaths->parallaxesPath = path(imgFolder).append("parallaxes");
    FOLDER_EXISTS(inputPaths->battlebacks1Path)
    FOLDER_EXISTS(inputPaths->battlebacks2Path)
    FOLDER_EXISTS(inputPaths->parallaxesPath)

    if (rpgMakerVersion == RPGMakerVersion::MV) {
        inputPaths->animationsPath = path(imgFolder).append("animations");
        FOLDER_EXISTS(inputPaths->animationsPath)
    } else {
        inputPaths->effectsPath = path(inputFolder).append("effects");
        FOLDER_EXISTS(inputPaths->effectsPath)
    }

    return true;
}
