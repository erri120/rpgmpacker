#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/fmt.h>
#include <ghc/filesystem.hpp>
#include <taskflow/taskflow.hpp>
#include <iostream>
#include <string>
#include <numeric>
#include "md5.h"

#include "platform.hpp"
#include "formatters.hpp"
#include "utility.hpp"
#include "foldertype.hpp"
#include "operation.hpp"

//#define RPGMPACKER_TESTING

//bool filterFolder(ghc::filesystem::path* from, ghc::filesystem::path* to, FolderType folderType, RPGMakerVersion version, Platform platform);
bool filterFile(ghc::filesystem::path* from, ghc::filesystem::path* to, FolderType folderType, RPGMakerVersion version, Platform platform);
bool shouldEncryptFile(ghc::filesystem::path* from, ghc::filesystem::path* to, bool encryptAudio, bool encryptImages, RPGMakerVersion version);

int main(int argc, char** argv) {
    auto logger = spdlog::stdout_color_mt("console");
    auto errorLogger = spdlog::stderr_color_mt("stderr");

    cxxopts::Options options("RPGMPacker", "RPGMaker Games Packer for use in a CI/CD workflow.");
    options.add_options()
        ("i,input", "(REQUIRED) Input folder containing the .rpgproj file", cxxopts::value<std::string>())
        ("o,output", "(REQUIRED) Output folder", cxxopts::value<std::string>())
        ("rpgmaker", "(REQUIRED) RPG Maker installation folder", cxxopts::value<std::string>())
        ("p,platforms", "(REQUIRED) Platforms to build for, this can take a list of platforms delimited with a comma or just one value. Possible values: win, osx, linux, browser, mobile", cxxopts::value<std::vector<std::string>>())
        ("encryptImages", "Enable Image Encryption using encryptionKey. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("encryptAudio", "Enable Audio Encryption using encryptionKey. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("encryptionKey", "Encryption Key for Images or Audio, either encryptImages or encryptAudio have to be set", cxxopts::value<std::string>())
        ("hardlinks", "Use hardlinks instead of creating copies. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("cache", "Use a path cache for already encrypted files when multi-targeting and using hardlinks. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("threads", "Amount of worker threads to use. Min: 1, Max: 10", cxxopts::value<int>()->default_value("2"))
        ("d,debug", "Enable debugging output (very noisy). (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");

    std::string input, output, rpgmaker, encryptionKey;
    bool encryptImages, encryptAudio, debug, useHardlinks, useCache;
    std::vector<std::string> platformNames;
    int workerThreads = 0;

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
        useCache = true;
        workerThreads = 2;

        platformNames = std::vector<std::string>();
        platformNames.emplace_back("win");
        //platformNames.emplace_back("osx");
        //platformNames.emplace_back("linux");
        platformNames.emplace_back("browser");
#else
        input = result["input"].as<std::string>();
        output = result["output"].as<std::string>();
        rpgmaker = result["rpgmaker"].as<std::string>();
        encryptImages = result["encryptImages"].as<bool>();
        encryptAudio = result["encryptAudio"].as<bool>();
        debug = result["debug"].as<bool>();
        useHardlinks = result["hardlinks"].as<bool>();
        useCache = result["cache"].as<bool>();
        workerThreads = result["threads"].as<int>();

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
    logger->info("Use Cache: {}", useCache);
    logger->info("Worker Threads: {}", workerThreads);

    if (!isValidDirectory(input, "Input", errorLogger))
        return -1;
    if (!isValidDirectory(rpgmaker, "RPG Maker", errorLogger))
        return -1;

    auto inputPath = ghc::filesystem::path(input);
    auto outputPath = ghc::filesystem::path(output);
    auto rpgmakerPath = ghc::filesystem::path(rpgmaker);

    auto rpgmakerVersion = getRPGMakerVersion(inputPath, logger, errorLogger);
    if (rpgmakerVersion == RPGMakerVersion::None)
        return -1;

    std::vector<Platform> platforms;
    if (!getPlatforms(&platformNames, &platforms, rpgmakerVersion, errorLogger))
        return -1;

    auto inputRootName = inputPath.root_name();
    auto outputRootName = outputPath.root_name();
    auto rpgmakerRootName = rpgmakerPath.root_name();

    auto canUseHardlinksRPGMakerToOutput = useHardlinks && rpgmakerRootName == outputRootName;
    auto canUseHardlinksInputToOutput = useHardlinks && inputRootName == outputRootName;

    if (useHardlinks) {
        logger->info("Can use hardlinking from RPGMaker Folder to Output: {}", canUseHardlinksRPGMakerToOutput);
        logger->info("Can use hardlinking from Input Folder to Output: {}", canUseHardlinksInputToOutput);
    }

    if (useCache && (!useHardlinks || (!encryptAudio && !encryptImages))) {
        logger->warn("Cache option is active but requires hardlink and encryption to be turned on as well, it will be disabled.");
        useCache = false;
    }

    if (workerThreads <= 0 || workerThreads > 10) {
        logger->warn("Worker Threads count is <= 0 or > 10, it will be set to 2.");
        workerThreads = 2;
    }

    MD5 md5;
    std::string encryptionHash = std::string("");
    if (encryptImages || encryptAudio)
        encryptionHash = md5(encryptionKey);

    auto hash = stringToHexHash(encryptionHash);

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

    if (!ensureDirectory(outputPath, errorLogger))
        return 1;

    tf::Executor executor(workerThreads);

    logger->info("Building output for {} platforms", platforms.size());
    for (auto platform : platforms) {
        spdlog::stopwatch sw;
        logger->debug("Platform {}", platform);
        auto platformOutputPath = ghc::filesystem::path(outputPath).append(PlatformNames[(int)platform]);
        if (!ensureDirectory(platformOutputPath, errorLogger))
            return 1;

        std::vector<Operation> operations;

        auto templateName = PlatformFolders[(int)platform];
        if (!templateName.empty()) {
            auto templateFolderPath = ghc::filesystem::path(rpgmakerPath).append(templateName);
            auto sTemplateFolderPath = std::string(templateFolderPath.c_str());

            if (!ghc::filesystem::exists(templateFolderPath, ec)) {
                errorLogger->error("The template directory at {} does not exist!", templateFolderPath);
                return 1;
            }

            logger->debug("Template Folder: {}", templateFolderPath);
            for (auto p : ghc::filesystem::recursive_directory_iterator(templateFolderPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
                auto path = p.path();
                auto sPath = std::string(path.c_str());
                auto outputPath = ghc::filesystem::path(platformOutputPath).append(sPath.substr(sTemplateFolderPath.length()+1));

                if (p.is_directory(ec)) {
                    if (!ensureDirectory(outputPath, errorLogger))
                        return 1;
                } else if (p.is_regular_file(ec)) {
                    if (filterFile(&path, &outputPath, FolderType::RPGMaker, rpgmakerVersion, platform))
                        continue;

                    struct Operation operation
                    {
                        path,
                        outputPath,
                        OperationType::Copy,
                        FolderType::RPGMaker
                    };

                    //if(!copyFile(path, outputPath, canUseHardlinksRPGMakerToOutput, logger, errorLogger))
                    //    return 1;
                    operations.emplace_back(operation);
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

        auto sInputPath = std::string(inputPath.c_str());
        for (auto p : ghc::filesystem::recursive_directory_iterator(inputPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
            auto path = p.path();
            auto sPath = std::string(path.c_str());
            auto outputPath = ghc::filesystem::path(wwwPath).append(sPath.substr(sInputPath.length()+1));

            if (p.is_directory(ec)) {
                if (!ensureDirectory(outputPath, errorLogger))
                    return 1;
            } else if (p.is_regular_file(ec)) {
                auto filename = path.filename();
                if (filterFile(&path, &outputPath, FolderType::Project, rpgmakerVersion, platform))
                    continue;

                struct Operation operation
                {
                    path,
                    outputPath,
                    OperationType::Copy,
                    FolderType::Project
                };

                if (encryptAudio || encryptImages) {
                    if (shouldEncryptFile(&path, &outputPath, encryptAudio, encryptImages, rpgmakerVersion)) {
                        operation.type = OperationType::Encrypt;
                    }

                    //updating System.json with the encryption data
                    if (filename == "System.json") {
                        if (!updateSystemJson(path, outputPath, encryptAudio, encryptImages, encryptionHash, logger, errorLogger))
                            return 1;
                        continue;
                    }
                }

                operations.emplace_back(operation);
            }
        }

        tf::Taskflow taskflow;

        taskflow.for_each(operations.begin(), operations.end(), [&](struct Operation& operation) {
            auto canUseHardlinks = operation.folderType == FolderType::Project
                ? canUseHardlinksInputToOutput
                : canUseHardlinksRPGMakerToOutput;
            if (operation.type == OperationType::Copy) {
                copyFile(operation.from, operation.to, canUseHardlinks, logger, errorLogger);
            } else if (operation.type == OperationType::Encrypt) {
                encryptFile(operation.from, operation.to, hash, useCache, canUseHardlinks, platform, rpgmakerVersion, logger, errorLogger);
            }
        });

        auto fu = executor.run(taskflow);
        fu.wait();

        logger->info("Finished operations for {} in {} seconds", platform, sw);
    }

    delete[] hash;
    return 0;
}

bool filterFile(ghc::filesystem::path* from, ghc::filesystem::path* to, FolderType folderType, RPGMakerVersion version, Platform platform) {
    auto filename = from->filename();
    auto extension = from->extension();

    auto parent = from->parent_path();
    auto parentName = parent.filename();

    if (folderType == FolderType::RPGMaker) {
        if (version == RPGMakerVersion::MZ) {
            if (parentName == "pnacl") return true;

            //nw.exe gets renamed to Game.exe to reduce confusion for players
            if (filename == "nw.exe") {
                to->replace_filename("Game.exe");
                return false;
            }

            //chromium related files that are ignored
            //TODO: Windows only?
            if (filename == "chromedriver.exe") return true;
            if (filename == "nacl_irt_x86_64.nexe") return true;
            if (filename == "nw.exe") return true;
            if (filename == "nwjc.exe") return true;
            if (filename == "payload.exe") return true;
        }
    } else if (folderType == FolderType::Project) {
        //Desktop: only ogg
        //Mobile: only m4a
        //Browser: both
        if (platform == Platform::Mobile && extension == ".ogg") return true;
        if (extension == ".m4a" && platform != Platform::Mobile && platform != Platform::Browser) return true;

        //skip project files
        if (extension == ".rpgproject") return true;
        if (extension == ".rmmzproject") return true;
    }

    return false;
}

bool shouldEncryptFile(ghc::filesystem::path* from, ghc::filesystem::path* to, bool encryptAudio, bool encryptImages, RPGMakerVersion version) {
    auto filename = from->filename();
    auto extension = from->extension();
    auto parent = from->parent_path();
    auto parentname = parent.filename();
    auto grandparent = parent.parent_path();
    auto grandparentname = grandparent.filename();

    if (extension != ".png" && extension != ".ogg" && extension != ".m4a")
        return false;

    if (encryptAudio && (extension == ".ogg" || extension == ".m4a")) {
        return true;
    }

    if (encryptImages && extension == ".png") {
        if (version == RPGMakerVersion::MZ) {
            //effects/Textures is not encrypted in MZ
            if (parentname == "Texture" && grandparentname == "effects")
                return false;

            //img/system gets encrypted, in MV some are left out
            if (parentname == "system" && grandparentname == "img")
                return true;
        } else if (version == RPGMakerVersion::MV) {
            if (parentname == "system" && grandparentname == "img") {
                //these are for some reason not encrypted in MV
                if (filename == "Loading.png") return false;
                if (filename == "Window.png") return false;
            }
        }

        //game icon for the window
        if (filename == "icon.png" && parentname == "icon")
            return false;

        return true;
    }

    return false;
}
