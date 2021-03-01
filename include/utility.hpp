#pragma once

#include <string>

#include <spdlog/spdlog.h>
#include <ghc/filesystem.hpp>

#include "formatters.hpp"
#include "loggers.h"
#include "foldertype.hpp"
#include "platform.hpp"
#include "rpgmakerVersion.hpp"
#include "parseData.hpp"
#include "inputPaths.hpp"

bool isValidDirectory(const std::string& directory, const std::string& name, const struct Loggers& loggers);
bool ensureDirectory(const ghc::filesystem::path& path, const struct Loggers& loggers);
bool getPlatforms(std::vector<std::string>* names, std::vector<Platform>* platforms, RPGMakerVersion version, const struct Loggers& loggers);
bool copyFile(const ghc::filesystem::path& from, const ghc::filesystem::path& to, bool useHardlinks, const struct Loggers& loggers);
unsigned int* stringToHexHash(std::string encryptionHash);
bool encryptFile(const ghc::filesystem::path& from, ghc::filesystem::path to, const unsigned int* encryptionHash, bool useCache, bool hardlink, Platform platform, RPGMakerVersion version, const struct Loggers& loggers);
bool updateSystemJson(const ghc::filesystem::path& from, const ghc::filesystem::path& to, bool encryptAudio, bool encryptImages, const std::string& hash, const struct Loggers& loggers);
RPGMakerVersion getRPGMakerVersion(const ghc::filesystem::path& projectPath, const struct Loggers& loggers);
bool filterFile(ghc::filesystem::path* from, ghc::filesystem::path* to, FolderType folderType, RPGMakerVersion version, Platform platform);
bool shouldEncryptFile(ghc::filesystem::path *from, bool encryptAudio, bool encryptImages, RPGMakerVersion version);
bool filterUnusedFiles(const ghc::filesystem::path& path, struct InputPaths* inputPaths, struct ParsedData* parsedData, RPGMakerVersion version);
std::string getPlatformFolder(RPGMakerVersion version, Platform platform);