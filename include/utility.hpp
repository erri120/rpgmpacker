#pragma once

#include <spdlog/spdlog.h>
#include <string>
#include <ghc/filesystem.hpp>

#include "formatters.hpp"
#include "platform.hpp"
#include "rpgmakerVersion.hpp"

bool isValidDirectory(const std::string& directory, const std::string& name, const std::shared_ptr<spdlog::logger>& errorLogger);
bool ensureDirectory(const ghc::filesystem::path& path, const std::shared_ptr<spdlog::logger>& errorLogger);
bool getPlatforms(std::vector<std::string>* names, std::vector<Platform>* platforms, RPGMakerVersion version, const std::shared_ptr<spdlog::logger>& errorLogger);
bool copyFile(const ghc::filesystem::path& from, const ghc::filesystem::path& to, bool useHardlinks, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger);
unsigned int* stringToHexHash(std::string encryptionHash);
bool encryptFile(const ghc::filesystem::path& from, ghc::filesystem::path to, const unsigned int* encryptionHash, bool useCache, bool hardlink, Platform platform, RPGMakerVersion version, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger);
bool updateSystemJson(const ghc::filesystem::path& from, const ghc::filesystem::path& to, bool encryptAudio, bool encryptImages, const std::string& hash, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger);
RPGMakerVersion getRPGMakerVersion(const ghc::filesystem::path& projectPath, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger);