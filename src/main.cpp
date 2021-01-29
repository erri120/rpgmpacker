#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/fmt.h>
#include <ghc/filesystem.hpp>
#include <iostream>
#include <string>
#include <numeric>
#include "md5.h"

#include "platform.hpp"
#include "formatters.hpp"
#include "utility.hpp"

#define RPGMPACKER_TESTING

int main(int argc, char** argv) {
    auto logger = spdlog::stdout_color_mt("console");
    auto errorLogger = spdlog::stderr_color_mt("stderr");

    cxxopts::Options options("RPGMPacker", "RPGMaker Games Packer for Continues Deployment.");
    options.add_options()
        ("i,input", "(REQUIRED) Input folder containing the .rpgproj file", cxxopts::value<std::string>())
        ("o,output", "(REQUIRED) Output folder", cxxopts::value<std::string>())
        ("rpgmaker", "(REQUIRED) RPG Maker installation folder", cxxopts::value<std::string>())
        ("p,platforms", "(REQUIRED) Platforms to build for, this can take a list of platforms delimited with a comma or just one value. Possible values: win, osx, linux, browser, mobile", cxxopts::value<std::vector<std::string>>())
        ("encryptImages", "Enable Image Encryption using encryptionKey. Default: false", cxxopts::value<bool>()->default_value("false"))
        ("encryptAudio", "Enable Audio Encryption using encryptionKey. Default: false", cxxopts::value<bool>()->default_value("false"))
        ("encryptionKey", "Encryption Key for Images or Audio, either encryptImages or encryptAudio have to be set", cxxopts::value<std::string>())
        ("hardlinks", "Use hardlinks instead of creating copies. Default: false", cxxopts::value<bool>()->default_value("false"))
        ("d,debug", "Enable debugging. Default: false", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");

    std::string input, output, rpgmaker, encryptionKey;
    bool encryptImages, encryptAudio, debug, useHardlinks;
    std::vector<std::string> platformNames;

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

#ifdef RPGMPACKER_TESTING
        //input = "E:\\Projects\\RPGMakerTest\\src\\Project1";
        input = "E:\\Projects\\RPGMakerTest\\src\\MZProject1";
        output = "E:\\Projects\\RPGMakerTest\\out-c";
        //rpgmaker = "M:\\SteamLibrary\\steamapps\\common\\RPG Maker MV";
        rpgmaker = "C:\\Program Files\\KADOKAWA\\RPGMZ";
        encryptImages = true;
        encryptAudio = true;
        encryptionKey = std::string("1337");
        //debug = true;
        debug = false;
        useHardlinks = true;

        platformNames = std::vector<std::string>();
        platformNames.emplace_back("win");
        //platformNames.emplace_back("osx");
        //platformNames.emplace_back("linux");
        //platformNames.emplace_back("browser");
#else
        input = result["input"].as<std::string>();
        output = result["output"].as<std::string>();
        rpgmaker = result["rpgmaker"].as<std::string>();
        encryptImages = result["encryptImages"].as<bool>();
        encryptAudio = result["encryptAudio"].as<bool>();
        debug = result["debug"].as<bool>();
        useHardlinks = result["hardlinks"].as<bool>();

        if (encryptImages || encryptAudio)
            encryptionKey = result["encryptionKey"].as<std::string>();
        else
            encryptionKey = std::string("");

        platformNames = result["platforms"].as<std::vector<std::string>>();
#endif
    } catch (const cxxopts::OptionException& e) {
        errorLogger->error(e.what());
        std::cout << options.help() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        errorLogger->error("Exception while parsing arguments: {}", e.what());
        std::cout << options.help() << std::endl;
        return -1;
    }

    if (debug) {
        logger->set_level(spdlog::level::debug);
        errorLogger->set_level(spdlog::level::debug);
    }

    //input dump
    logger->info("Input: {}", input);
    logger->info("Output: {}", output);
    logger->info("RPG Maker: {}", rpgmaker);
    if (encryptionKey.empty())
        logger->info("Encryption Key: NONE");
    else
        logger->info("Encryption Key: REDACTED");
    logger->info("Encrypt Images: {}", encryptImages);
    logger->info("Encrypt Audio: {}", encryptAudio);
    auto strReduce = [](std::string a, std::string b) {
        return std::move(a) + ',' + b;
    };
    auto platformsStr = std::accumulate(std::next(platformNames.begin()), platformNames.end(), platformNames.at(0), strReduce);
    logger->info("Platforms: {}", platformsStr);
    logger->info("Debug: {}", debug);
    logger->info("Use Hardlinks: {}", useHardlinks);

    if (!isValidDirectory(input, "Input", errorLogger))
        return -1;
    if (!isValidDirectory(rpgmaker, "RPG Maker", errorLogger))
        return -1;

    std::vector<Platform> platforms;
    if (!getPlatforms(&platformNames, &platforms, errorLogger))
        return -1;

    auto inputPath = ghc::filesystem::path(input);
    auto outputPath = ghc::filesystem::path(output);
    auto rpgmakerPath = ghc::filesystem::path(rpgmaker);

    auto inputRootName = inputPath.root_name();
    auto outputRootName = outputPath.root_name();
    auto rpgmakerRootName = rpgmakerPath.root_name();

    auto canUseHardlinksRPGMakerToOutput = useHardlinks && rpgmakerRootName == outputRootName;
    auto canUseHardlinksInputToOutput = useHardlinks && inputRootName == outputRootName;

    MD5 md5;
    std::string encryptionHash = std::string("");
    if (encryptImages || encryptAudio)
        encryptionHash = md5(encryptionKey);

    auto hash = stringToHexHash(encryptionHash);

    auto rpgmakerVersion = getRPGMakerVersion(inputPath, logger, errorLogger);
    if (rpgmakerVersion == RPGMakerVersion::None)
        return -1;

    std::error_code ec;
    if (ghc::filesystem::exists(outputPath, ec)) {
        spdlog::stopwatch sw;
        logger->info("Output Folder exists, removing old files...");
        auto result = ghc::filesystem::remove_all(outputPath, ec);
        if (ec) {
            logger->warn("Unable to completely remove the output directory! {}", ec);
        }
        logger->info("Finished cleaning the output directory in {} seconds deleting {} things", sw, result);
    }

    ghc::filesystem::create_directory(outputPath, ec);
    if (!ensureDirectory(outputPath, errorLogger))
        return 1;

    logger->info("Building output for {} platforms", platforms.size());
    for (auto platform : platforms) {
        spdlog::stopwatch sw;
        logger->debug("Platform {}", platform);
        auto platformOutputPath = ghc::filesystem::path(outputPath).append(PlatformNames[(int)platform]);
        if (!ensureDirectory(platformOutputPath, errorLogger))
            return 1;

        auto templateName = PlatformFolders[(int)platform];
        if (!templateName.empty()) {
            auto templateFolderPath = ghc::filesystem::path(rpgmakerPath).append(templateName);
            if (!ghc::filesystem::exists(templateFolderPath, ec)) {
                errorLogger->error("The template directory at {} does not exist!", templateFolderPath);
                return 1;
            }

            logger->debug("Template Folder: {}", templateFolderPath);
            logger->info("Copying files from {} to {}", templateFolderPath, platformOutputPath);
            for (auto p : ghc::filesystem::directory_iterator(templateFolderPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
                auto path = p.path();
                logger->debug("{}", path);

                if (p.is_directory(ec)) {
                    auto dirname = path.filename();
                    auto outputDirPath = ghc::filesystem::path(platformOutputPath).append(dirname);

                    //MZ only: pnacl folder gets ignored
                    if (rpgmakerVersion == RPGMakerVersion::MZ && dirname == "pnacl")
                        continue;

                    logger->debug("Copying folder from {} to {}", path, outputDirPath);
                    ghc::filesystem::copy(path, outputDirPath, ghc::filesystem::copy_options::recursive |
                        ghc::filesystem::copy_options::overwrite_existing, ec);
                    if (ec) {
                        errorLogger->error("Unable to copy folder from {} to {}! {}", path, outputDirPath, ec);
                        ec.clear();
                    }
                } else if (p.is_regular_file(ec)) {
                    auto filename = path.filename();
                    //MZ onyl: nw.exe gets renamed to Game.exe to reduce confusion for players
                    if (rpgmakerVersion == RPGMakerVersion::MZ && filename == "nw.exe")
                        filename = ghc::filesystem::path("Game.exe");
                    auto outputFilePath = ghc::filesystem::path(platformOutputPath).append(filename);

                    //MZ only: those files get ignored
                    if (rpgmakerVersion == RPGMakerVersion::MZ &&
                        (filename == "chromedriver.exe"
                            || filename == "nacl_irt_x86_64.nexe"
                            || filename == "nw.exe"
                            || filename == "nwjc.exe"
                            || filename == "payload.exe"))
                        continue;

                    if(!copyFile(path, outputFilePath, canUseHardlinksRPGMakerToOutput, logger, errorLogger))
                        return 1;
                }
            }
        }

        //MV has a www folder, MZ does not
        auto wwwPath = rpgmakerVersion == RPGMakerVersion::MV
            ? platform == Platform::OSX
                ? ghc::filesystem::path(platformOutputPath).append("Game.app/Contents/Resources/app.nw")
                : ghc::filesystem::path(platformOutputPath).append("www")
            : ghc::filesystem::path(platformOutputPath);
        if (!ensureDirectory(wwwPath, errorLogger))
            return 1;

        logger->info("Copying files from {} to {}", inputPath, wwwPath);
        for (auto p : ghc::filesystem::directory_iterator(inputPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
            auto path = p.path();
            logger->debug("{}", path);
            if (p.is_directory(ec)) {
                auto dirname = path.filename();
                auto outputDirPath = ghc::filesystem::path(wwwPath).append(dirname);
                if (!ensureDirectory(outputDirPath, errorLogger))
                    return 1;

                logger->debug("Copying folder from {} to {}", path, outputDirPath);
                for (auto f : ghc::filesystem::recursive_directory_iterator(path, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
                    if (f.is_directory(ec)) {
                        auto dirpath = f.path();
                        auto dirOutputPath = ghc::filesystem::path(outputDirPath).append(
                            std::string(dirpath.c_str())
                            .substr(std::string(path.c_str()).length()+1));
                        if (!ensureDirectory(dirOutputPath, errorLogger))
                            return 1;

                        continue;
                    }
                    auto filepath = f.path();
                    auto filename = filepath.filename();
                    auto extension = filepath.extension();

                    //Desktop: only ogg
                    //Mobile: only m4a
                    //Browser: both
                    if (platform == Platform::Mobile && extension == ".ogg") continue;
                    if (extension == ".m4a" && platform != Platform::Mobile && platform != Platform::Browser) continue;

                    auto outputFilePath = ghc::filesystem::path(outputDirPath).append(
                        std::string(filepath.c_str())
                        .substr(std::string(path.c_str()).length()+1));
                    auto outputFileDirectory = outputFilePath.parent_path();
                    if (!ensureDirectory(outputFileDirectory, errorLogger))
                        return 1;

                    if (encryptAudio || encryptImages) {
                        if (encryptImages && extension == ".png") {
                            auto dirname = filepath.parent_path().filename();
                            auto parentdirname = filepath.parent_path().parent_path().filename();
                            //MZ only: effects/Texture is not being encrypted
                            if (rpgmakerVersion == RPGMakerVersion::MZ && parentdirname == "effects" && dirname == "Texture") {
                                if(!copyFile(filepath, outputFilePath, canUseHardlinksInputToOutput, logger, errorLogger))
                                    return 1;
                                continue;
                            }

                            //MZ only: Window.png will also be encrypted, in MV this was not the case
                            if (rpgmakerVersion == RPGMakerVersion::MZ && filename == "Window.png") {
                                if (!encryptFile(filepath, outputFilePath, hash, rpgmakerVersion, logger, errorLogger))
                                    return 1;
                                continue;
                            }
                            //icon.png, Loading.png and Window.png must not be encrypted because they are not
                            //getting decrypted
                            if (filename != "icon.png" && filename != "Loading.png" && filename != "Window.png") {
                                if (!encryptFile(filepath, outputFilePath, hash, rpgmakerVersion, logger, errorLogger))
                                    return 1;
                                continue;
                            }
                        } else if (encryptAudio && extension == ".ogg" || extension == ".m4a") {
                            if (!encryptFile(filepath, outputFilePath, hash, rpgmakerVersion, logger, errorLogger))
                                return 1;
                            continue;
                        }

                        //updating System.json with the encryption data
                        if (filename == "System.json") {
                            if (!updateSystemJson(filepath, outputFilePath, encryptAudio, encryptImages, encryptionHash, logger, errorLogger))
                                return 1;
                            continue;
                        }
                    }

                    if(!copyFile(filepath, outputFilePath, canUseHardlinksInputToOutput, logger, errorLogger))
                        return 1;
                }
            } else if (p.is_regular_file(ec)) {
                auto filename = path.filename();
                auto extension = path.extension();
                //don't ship project files, maybe also skip package.json
                if (extension == ".rpgproject") continue;
                if (extension == ".rmmzproject") continue;

                auto outputFilePath = ghc::filesystem::path(wwwPath).append(filename);

                if(!copyFile(path, outputFilePath, canUseHardlinksInputToOutput, logger, errorLogger))
                    return 1;
            }
        }
        logger->info("Finished copying files for {} in {} seconds", platform, sw);
    }

    delete[] hash;
    return 0;
}
