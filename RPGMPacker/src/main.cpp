#include <iostream>
#include <string>
#include <numeric>

#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/stopwatch.h>
#include <ghc/filesystem.hpp>
#include <taskflow/taskflow.hpp>
#include <toml.hpp>
#include <md5.h>

#include <loggers.h>
#include <platform.hpp>
#include <formatters.hpp>
#include <utility.hpp>
#include <foldertype.hpp>
#include <operation.hpp>
#include <parseData.hpp>
#include <inputPaths.hpp>

int main(int argc, char** argv) {
    std::error_code ec;
    auto logger = spdlog::stdout_color_mt("console");
    auto errorLogger = spdlog::stderr_color_mt("stderr");
    const struct Loggers loggers(logger, errorLogger);

    cxxopts::Options options("RPGMPacker", "RPGMaker Games Packer for use in a CI/CD workflow.");
    options.add_options()
        ("config", "Config file location if the config file is not named config.toml", cxxopts::value<std::string>()->default_value(""))
        ("i,input", "(REQUIRED) Input folder containing the .rpgproj file", cxxopts::value<std::string>())
        ("o,output", "(REQUIRED) Output folder", cxxopts::value<std::string>())
        ("rpgmaker", "(REQUIRED) RPG Maker installation folder", cxxopts::value<std::string>())
        ("p,platforms", "(REQUIRED) Platforms to build for, this can take a list of platforms delimited with a comma or just one value. Possible values: win, osx, linux, browser, mobile", cxxopts::value<std::vector<std::string>>())
        ("encryptImages", "Enable Image Encryption using encryptionKey. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("encryptAudio", "Enable Audio Encryption using encryptionKey. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("encryptionKey", "Encryption Key for Images or Audio, either encryptImages or encryptAudio have to be set", cxxopts::value<std::string>())
        ("exclude", "Exclude unused files. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("hardlinks", "Use hardlinks instead of creating copies. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("cache", "Use a path cache for already encrypted files when multi-targeting and using hardlinks. (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("threads", "Amount of worker threads to use. Min: 1, Max: 10", cxxopts::value<int>()->default_value("2"))
        ("d,debug", "Enable debugging output (very noisy). (default: false)", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");

    std::string input, output, rpgmaker, encryptionKey;
    bool encryptImages, encryptAudio, debug, useHardlinks, useCache, excludeUnused;
    std::vector<std::string> platformNames;
    int workerThreads = 0;

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return EXIT_SUCCESS;
        }

        auto configFile = result["config"].as<std::string>();
        auto defaultConfigPath = ghc::filesystem::path("config.toml");
        if (ghc::filesystem::is_regular_file(defaultConfigPath, ec) || !configFile.empty()) {
            configFile = configFile.empty() ? "config.toml" : configFile;
            auto data = toml::parse(configFile);
            auto tomlConfig = toml::find(data, "config");

            input = toml::find<std::string>(tomlConfig, "input");
            output = toml::find<std::string>(tomlConfig, "output");
            rpgmaker = toml::find<std::string>(tomlConfig, "rpgmaker");

#define TOML_GET(var, name, type, defaultValue) if (tomlConfig.contains(name)) { \
var = toml::find<type>(tomlConfig, name);\
} else { var = defaultValue; }\

            TOML_GET(encryptImages, "encryptImages", bool, false)
            TOML_GET(encryptAudio, "encryptAudio", bool, false)
            TOML_GET(excludeUnused, "exclude", bool, false)
            TOML_GET(debug, "debug", bool, false)
            TOML_GET(useHardlinks, "hardlinks", bool, false)
            TOML_GET(useCache, "cache", bool, false)
            TOML_GET(workerThreads, "threads", int64_t, 2)

            if (encryptImages || encryptAudio)
                encryptionKey = toml::find<std::string>(tomlConfig, "encryptionKey");
            else
                encryptionKey = "";

            platformNames = toml::find<std::vector<std::string>>(tomlConfig, "platforms");
        } else {
            input = result["input"].as<std::string>();
            output = result["output"].as<std::string>();
            rpgmaker = result["rpgmaker"].as<std::string>();
            encryptImages = result["encryptImages"].as<bool>();
            encryptAudio = result["encryptAudio"].as<bool>();
            excludeUnused = result["exclude"].as<bool>();
            debug = result["debug"].as<bool>();
            useHardlinks = result["hardlinks"].as<bool>();
            useCache = result["cache"].as<bool>();
            workerThreads = result["threads"].as<int>();

            if (encryptImages || encryptAudio)
                encryptionKey = result["encryptionKey"].as<std::string>();
            else
                encryptionKey = "";

            platformNames = result["platforms"].as<std::vector<std::string>>();
        }
    } catch (const cxxopts::OptionException& e) {
        errorLogger->error(e.what());
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    } catch (const toml::exception& e) {
        errorLogger->error("TOML exception: \n{}", e.what());
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        errorLogger->error("Runtime error: \n{}", e.what());
        return EXIT_FAILURE;
    } catch (const std::out_of_range& e) {
        errorLogger->error("Out of Range exception, most likely due to a bad config file: \n{}", e.what());
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        errorLogger->error("Exception while parsing arguments: \n{}", e.what());
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
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
    logger->info("Exclude Unused Files: {}", excludeUnused);
    auto strReduce = [](std::string a, std::string b) {
        return std::move(a) + ',' + std::move(b);
    };
    auto platformsStr = std::accumulate(std::next(platformNames.begin()), platformNames.end(), platformNames.at(0), strReduce);
    logger->info("Platforms: {}", platformsStr);
    logger->info("Debug: {}", debug);
    logger->info("Use Hardlinks: {}", useHardlinks);
    logger->info("Use Cache: {}", useCache);
    logger->info("Worker Threads: {}", workerThreads);

    if (!isValidDirectory(input, "Input", loggers))
        return EXIT_FAILURE;
    if (!isValidDirectory(rpgmaker, "RPG Maker", loggers))
        return EXIT_FAILURE;

    auto inputPath = ghc::filesystem::path(input);
    auto outputPath = ghc::filesystem::path(output);
    auto rpgmakerPath = ghc::filesystem::path(rpgmaker);

    auto rpgmakerVersion = getRPGMakerVersion(inputPath, loggers);
    if (rpgmakerVersion == RPGMakerVersion::None)
        return EXIT_FAILURE;

    std::vector<Platform> platforms;
    if (!getPlatforms(&platformNames, &platforms, rpgmakerVersion, loggers))
        return EXIT_FAILURE;

    if (rpgmakerVersion == RPGMakerVersion::MZ) {
        auto it = std::find(platforms.begin(), platforms.end(), Platform::Linux);
        if (it != platforms.end()) {
            errorLogger->error("Linux is not supported as an export target for MZ!");
            return EXIT_FAILURE;
        }
    } else {
        auto it = std::find(platforms.begin(), platforms.end(), Platform::Mobile);
        if (it != platforms.end()) {
            errorLogger->error("Mobile is not supported as an export target for MV using this tool!");
            return EXIT_FAILURE;
        }
    }

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

    if (ghc::filesystem::exists(outputPath, ec)) {
        spdlog::stopwatch sw;
        logger->info("Output Folder exists, removing old files...");
        auto result = ghc::filesystem::remove_all(outputPath, ec);
        if (ec) {
            errorLogger->error("Unable to completely remove the output directory! {}", ec);
            return EXIT_FAILURE;
        }

        logger->info("Finished cleaning the output directory in {} seconds deleting {} things", sw, result);
    }

    if (!ensureDirectory(outputPath, loggers))
        return EXIT_FAILURE;

    tf::Executor executor(workerThreads);

    struct ParsedData parsedData;
    struct InputPaths inputPaths;

    if (!getInputPaths(inputPath, &inputPaths, rpgmakerVersion, loggers))
        return EXIT_FAILURE;

    if (excludeUnused) {
        auto dataFolder = ghc::filesystem::path(inputPath).append("data");
        if (!ghc::filesystem::is_directory(dataFolder, ec)) {
            errorLogger->error("Directory does not exist: {}! {}", dataFolder, ec);
            return EXIT_FAILURE;
        }

        if (!parseData(dataFolder, &parsedData, rpgmakerVersion, loggers)) {
            return EXIT_FAILURE;
        }
    }

    logger->info("Building output for {} platforms", platforms.size());
    for (auto platform : platforms) {
        spdlog::stopwatch sw;
        logger->debug("Platform {}", platform);
        auto platformOutputPath = ghc::filesystem::path(outputPath).append(PlatformNames[(int)platform]);
        if (!ensureDirectory(platformOutputPath, loggers))
            return EXIT_FAILURE;

        std::vector<Operation> operations;

        auto templateName = getPlatformFolder(rpgmakerVersion, platform);
        if (!templateName.empty()) {
            auto templateFolderPath = ghc::filesystem::path(rpgmakerPath).append(templateName);
            auto sTemplateFolderPath = templateFolderPath.wstring();

            if (!ghc::filesystem::exists(templateFolderPath, ec)) {
                errorLogger->error("The template directory at {} does not exist!", templateFolderPath);
                return EXIT_FAILURE;
            }

            logger->debug("Template Folder: {}", templateFolderPath);
            for (const auto& p : ghc::filesystem::recursive_directory_iterator(templateFolderPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
                auto path = p.path();
                auto sPath = path.wstring();
                auto entryOutputPath = ghc::filesystem::path(platformOutputPath).append(sPath.substr(sTemplateFolderPath.length()+1));

                if (rpgmakerVersion == RPGMakerVersion::MZ && platform == Platform::OSX) {
                    auto sEntryOutputPath = entryOutputPath.wstring();
                    auto it = sEntryOutputPath.find(L"nwjs.app");
                    if (it != std::string::npos) {
                        entryOutputPath = ghc::filesystem::path(sEntryOutputPath.replace(it, 8, L"Game.app"));
                    }
                }

                if (p.is_directory(ec)) {
                    if (!ensureDirectory(entryOutputPath, loggers))
                        return EXIT_FAILURE;
                } else if (p.is_regular_file(ec)) {
                    if (filterFile(&path, &entryOutputPath, FolderType::RPGMaker, rpgmakerVersion, platform))
                        continue;

                    struct Operation operation
                    {
                        path,
                        entryOutputPath,
                        OperationType::Copy,
                        FolderType::RPGMaker
                    };

                    operations.push_back(operation);
                }
            }
        }

        //MV has a www folder, MZ does not
        auto wwwPath = platform == Platform::OSX
                ? ghc::filesystem::path(platformOutputPath).append("Game.app/Contents/Resources/app.nw")
                : rpgmakerVersion == RPGMakerVersion::MV
                    ? ghc::filesystem::path(platformOutputPath).append("www")
                    : ghc::filesystem::path(platformOutputPath);
        if (!ensureDirectory(wwwPath, loggers))
            return EXIT_FAILURE;

        auto sInputPath = inputPath.wstring();
        for (const auto& p : ghc::filesystem::recursive_directory_iterator(inputPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
            auto path = p.path();
            auto sPath = path.wstring();
            auto entryOutputPath = ghc::filesystem::path(wwwPath).append(sPath.substr(sInputPath.length()+1));

            if (p.is_directory(ec)) {
                if (!ensureDirectory(entryOutputPath, loggers))
                    return EXIT_FAILURE;
            } else if (p.is_regular_file(ec)) {
                auto filename = path.filename().u8string();

                if (excludeUnused) {
                    if (filterUnusedFiles(path, &inputPaths, &parsedData, rpgmakerVersion))
                        continue;
                }

                if (filterFile(&path, &entryOutputPath, FolderType::Project, rpgmakerVersion, platform))
                    continue;

                struct Operation operation
                {
                    path,
                    entryOutputPath,
                    OperationType::Copy,
                    FolderType::Project
                };

                if (encryptAudio || encryptImages) {
                    if (shouldEncryptFile(&path, encryptAudio, encryptImages, rpgmakerVersion)) {
                        operation.type = OperationType::Encrypt;
                    }

                    //updating System.json with the encryption data
                    if (filename == "System.json") {
                        if (!updateSystemJson(path, entryOutputPath, encryptAudio, encryptImages, encryptionHash, loggers))
                            return EXIT_FAILURE;
                        continue;
                    }
                }

                operations.push_back(operation);
            }
        }

        std::atomic<unsigned int> succeeded(0);
        std::atomic<unsigned int> failed(0);

        tf::Taskflow taskflow;

        taskflow.for_each(operations.begin(), operations.end(), [&](struct Operation& operation) {
            auto canUseHardlinks = operation.folderType == FolderType::Project
                ? canUseHardlinksInputToOutput
                : canUseHardlinksRPGMakerToOutput;
            if (operation.type == OperationType::Copy) {
                if(copyFile(operation.from, operation.to, canUseHardlinks, loggers)) {
                    succeeded++;
                } else {
                    failed++;
                }
            } else if (operation.type == OperationType::Encrypt) {
                if(encryptFile(operation.from, operation.to, hash, useCache, canUseHardlinks, platform, rpgmakerVersion, loggers)) {
                    succeeded++;
                } else {
                    failed++;
                }
            }
        });

        auto fu = executor.run(taskflow);
        fu.wait();

        if (failed.load() == 0) {
            logger->info("Successfully executed {} operations for {} in {} seconds", succeeded.load(), platform, sw);
        } else {
            errorLogger->error("Some operations failed for {} in {} seconds: {} failed, {} succeeded", platform, sw, failed.load(), succeeded.load());
            return EXIT_FAILURE;
        }
    }

    delete[] hash;
    return EXIT_SUCCESS;
}
