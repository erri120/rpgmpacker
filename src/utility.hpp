#pragma once
#include <spdlog/spdlog.h>
#include <ghc/filesystem.hpp>

#include "platform.hpp"

bool isValidDirectory(std::string directory, std::string name, std::shared_ptr<spdlog::logger> errorLogger) {
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

bool ensureDirectory(ghc::filesystem::path path, std::shared_ptr<spdlog::logger> errorLogger) {
    std::error_code ec;
    if (ghc::filesystem::exists(path, ec)) return true;
    if (ec) return false;

    if (ghc::filesystem::create_directory(path, ec)) return true;
    errorLogger->error("Unable to create directory {} {}", path, ec);
    return false;
}

bool getPlatforms(std::vector<std::string>* names, std::vector<Platform>* platforms, std::shared_ptr<spdlog::logger> errorLogger) {
    for (auto i = 0; i < names->size(); i++) {
        auto current = names->at(i);
        if (current == "win") {
            platforms->emplace_back(Platform::Windows);
            continue;
        }

        if (current == "osx") {
            platforms->emplace_back(Platform::OSX);
            continue;
        }

        if (current == "linux") {
            platforms->emplace_back(Platform::Linux);
            continue;
        }

        if (current == "browser") {
            platforms->emplace_back(Platform::Browser);
            continue;
        }

        if (current == "mobile") {
            //platforms->emplace_back(Platform::Mobile);
            //continue;
            errorLogger->error("Mobile is not supported at the moment!");
            return false;
        }

        errorLogger->error("Unknown platform: {}", current);
        return false;
    }

    return true;
}

bool copyFile(ghc::filesystem::path from, ghc::filesystem::path to, bool useHardlinks, std::shared_ptr<spdlog::logger> logger, std::shared_ptr<spdlog::logger> errorLogger) {
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
