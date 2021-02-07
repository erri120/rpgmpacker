#pragma once
#include <spdlog/spdlog.h>
#include <ghc/filesystem.hpp>
#include <string>
#include <map>

#include "utility.hpp"
#include "platform.hpp"
#include "rpgmakerVersion.hpp"

bool isValidDirectory(const std::string& directory, const std::string& name, const std::shared_ptr<spdlog::logger>& errorLogger) {
    if (!ghc::filesystem::exists(directory)) {
        errorLogger->error("{} Folder does not exist!", name);
        return false;
    }

    if (!ghc::filesystem::is_directory(directory)) {
        errorLogger->error("{} Folder is not a directory!", name);
        return false;
    }

    return true;
}

bool ensureDirectory(const ghc::filesystem::path& path, const std::shared_ptr<spdlog::logger>& errorLogger) {
    std::error_code ec;
    if (ghc::filesystem::exists(path, ec)) return true;
    if (ec) return false;

    if (ghc::filesystem::create_directory(path, ec)) return true;
    errorLogger->error("Unable to create directory {}! {}", path, ec);
    return false;
}

bool getPlatforms(std::vector<std::string>* names, std::vector<Platform>* platforms, RPGMakerVersion version, const std::shared_ptr<spdlog::logger>& errorLogger) {
    for (const auto& current : *names) {
        if (current == "win") {
            platforms->emplace_back(Platform::Windows);
            continue;
        }

        if (current == "osx") {
            platforms->emplace_back(Platform::OSX);
            continue;
        }

        if (current == "linux") {
            if (version == RPGMakerVersion::MZ) {
                errorLogger->error("Linux is not supported in MZ!");
                return false;
            }
            platforms->emplace_back(Platform::Linux);
            continue;
        }

        if (current == "browser") {
            platforms->emplace_back(Platform::Browser);
            continue;
        }

        if (current == "mobile") {
            if (version == RPGMakerVersion::MV) {
                errorLogger->error("Mobile is not supported for MV at the moment!");
                return false;
            }
            platforms->emplace_back(Platform::Mobile);
            continue;
        }

        errorLogger->error("Unknown platform: {}", current);
        return false;
    }

    return true;
}

bool copyFile(const ghc::filesystem::path& from, const ghc::filesystem::path& to, bool useHardlinks, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger) {
    if (useHardlinks) {
        logger->debug("Creating hardlink from {} to {}", from , to);
    } else {
        logger->debug("Copying file from {} to {}", from , to);
    }

    std::error_code ec;
    auto options = ghc::filesystem::copy_options::none;
    if (useHardlinks) {
        options |= ghc::filesystem::copy_options::create_hard_links;
    } else {
        options |= ghc::filesystem::copy_options::update_existing;
    }

    if (useHardlinks) {
        if (ghc::filesystem::exists(to, ec)) {
            if (ghc::filesystem::is_regular_file(to, ec)) {
                auto result = ghc::filesystem::remove(to, ec);
                if (ec || !result) {
                    errorLogger->error("Unable to delete file for hardlink at {}! {}", to, ec);
                    return false;
                }
            }
        }
    }

    ghc::filesystem::copy(from, to, options, ec);
    if (ec) {
        if (useHardlinks) {
            errorLogger->error("Unable to create hardlink from {} to {}! {}", from, to, ec);
        } else {
            errorLogger->error("Unable to copy file from {} to {}! {}", from, to, ec);
        }

        return false;
    }

    return true;
}

unsigned int* stringToHexHash(std::string encryptionHash) {
    auto result = new unsigned int[16];
    if (encryptionHash.empty())
        return result;

    for (auto i = 0, j = 0; i < 32; i += 2, j++) {
        auto c1 = encryptionHash[i];
        auto c2 = encryptionHash[i+1];

        auto c = std::string("");
        c.push_back(c1);
        c.push_back(c2);

        auto x = std::stoul(c, nullptr, 16);
        result[j] = x;
    }

    return result;
}

auto lastPlatform = Platform::None;
std::map<std::wstring, std::wstring> cachedEncryptedFiles;

static uint8_t header[] = { 0x52, 0x50, 0x47, 0x4D, 0x56, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };

bool encryptFile(const ghc::filesystem::path& from, ghc::filesystem::path to, const unsigned int* encryptionHash, bool useCache, bool hardlink, Platform platform, RPGMakerVersion version, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger) {
    auto extension = from.extension();

    if (extension == ".ogg") {
        if (version == RPGMakerVersion::MV)
            to.replace_extension(".rpgmvo");
        else
            to.replace_extension(".ogg_");
    } else if (extension == ".m4a") {
        if (version == RPGMakerVersion::MV)
            to.replace_extension(".rpgmvm");
        else
            to.replace_extension(".m4a_");
    } else if (extension == ".png") {
        if (version == RPGMakerVersion::MV)
            to.replace_extension(".rpgmvp");
        else
            to.replace_extension(".png_");
    } else {
        errorLogger->error("Unknown extension {} of file {}", extension, from);
        return false;
    }

    if (useCache) {
        if (lastPlatform == Platform::None)
            lastPlatform = platform;
        else if (lastPlatform != platform) {
            //hardlink to the previously encrypted files
            if (hardlink) {
                auto iter = cachedEncryptedFiles.find(from.wstring());
                if (iter != cachedEncryptedFiles.end()) {
                    auto prev = ghc::filesystem::path(iter->second);

                    std::error_code ec;
                    if (ghc::filesystem::exists(prev, ec)) {
                        logger->debug("Using cached path from encryption file at {} and hardlink to {}", prev, to);
                        if(copyFile(prev, to, true, logger, errorLogger))
                            return true;
                        return false;
                    } else {
                        logger->warn("Cached path {} does not exist anymore!", prev);
                        cachedEncryptedFiles.erase(iter);
                    }
                } else {
                    logger->warn("Unable to find path {} in cached map!", from);
                }
            }
        }
    }

    logger->debug("Encrypting file at {} to {}", from, to);

    auto ifstream = ghc::filesystem::ifstream(from, std::ios_base::in | std::ios_base::binary);
    if (!ifstream.is_open()){
        errorLogger->error("Unable to open file {}", from);
        return false;
    }

    auto ofstream = ghc::filesystem::ofstream(to, std::ios_base::out | std::ios_base::binary);
    if (!ofstream.is_open()){
        errorLogger->error("Unable to open file {}", to);
        ifstream.close();
        return false;
    }

    ifstream.seekg(0, std::ios_base::end);
    auto filelength = ifstream.tellg();
    ifstream.seekg(0, std::ios_base::beg);

    if (filelength < 16) {
        errorLogger->error("File {} is less than 16 bytes long: {}", from, filelength);
        return false;
    }

    /*
        "Encryption" in RPG Maker:
        1) write a header
        2) encrypt the first 16 bytes from the original file using a simple XOR operation
        3) write the rest of the original file
    */

    ofstream.write((char*)&header, 16);

    auto bytes = new char[16];
    ifstream.read(bytes, 16);

    for (auto i = 0; i < 16; i++) {
        auto c1 = bytes[i];
        auto c2 = encryptionHash[i];
        auto result = c1 ^ c2;
        bytes[i] = (char)result;
    }
    ofstream.write(bytes, 16);

    auto buffer = new char[4096];
    auto max = (long)filelength - 4096;
    while (ifstream.tellg() < max) {
        ifstream.read(buffer, 4096);
        ofstream.write(buffer, 4096);
    }

    if (ifstream.tellg() != filelength) {
        auto left = (long)(filelength - ifstream.tellg());
        ifstream.read(buffer, left);
        ofstream.write(buffer, left);
    }

    ifstream.close();
    ofstream.close();
    delete[] bytes;
    delete[] buffer;

    cachedEncryptedFiles[from.wstring()] = to.wstring();

    return true;
}

bool updateSystemJson(const ghc::filesystem::path& from, const ghc::filesystem::path& to, bool encryptAudio, bool encryptImages, const std::string& hash, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger) {
    logger->debug("Updating System.json with encryption data from {} to {}", from, to);

    auto ifstream = ghc::filesystem::ifstream(from, std::ios_base::in);
    if (!ifstream.is_open()){
        errorLogger->error("Unable to open file {}", from);
        return false;
    }

    auto ofstream = ghc::filesystem::ofstream(to, std::ios_base::out);
    if (!ofstream.is_open()){
        errorLogger->error("Unable to open file {}", to);
        ifstream.close();
        return false;
    }

    ifstream.seekg(0, std::ios_base::end);
    auto filelength = (long)ifstream.tellg();
    ifstream.seekg(0, std::ios_base::beg);

    auto buffer = new char[filelength];
    ifstream.read(buffer, filelength);
    ofstream.write(buffer, filelength-1);

    std::string json(",\"hasEncryptedImages\":");
    if (encryptImages)
        json.append("true");
    else
        json.append("false");
    json.append(",\"hasEncryptedAudio\":");
    if (encryptAudio)
        json.append("true");
    else
        json.append("false");
    json.append(R"(,"encryptionKey":")");
    json.append(hash);
    json.append("\"}");

    ofstream.write(json.c_str(), json.length());

    ifstream.close();
    ofstream.close();

    return true;
}

RPGMakerVersion getRPGMakerVersion(const ghc::filesystem::path& projectPath, const std::shared_ptr<spdlog::logger>& logger, const std::shared_ptr<spdlog::logger>& errorLogger) {
    logger->debug("Identifying RPGMaker Version");

    std::error_code ec;
    auto version = RPGMakerVersion::None;
    for (const auto& p : ghc::filesystem::directory_iterator(projectPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
        if (!p.is_regular_file()) continue;
        auto extension = p.path().extension();

        if (extension == ".rpgproject") {
            version = RPGMakerVersion::MV;
            break;
        } else if (extension == ".rmmzproject") {
            version = RPGMakerVersion::MZ;
            break;
        }
    }

    if (version == RPGMakerVersion::None) {
        errorLogger->error("Unable to identify RPGMaker Version!");
    } else {
        logger->info("RPGMaker Version: {}", version);
    }

    return version;
}