#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/stopwatch.h>
#include <ghc/filesystem.hpp>
#include <taskflow/taskflow.hpp>
#include <simdjson.h>
#include <iostream>
#include <string>
#include <numeric>
#include "md5.h"

#include "platform.hpp"
#include "formatters.hpp"
#include "utility.hpp"
#include "foldertype.hpp"
#include "operation.hpp"

//bool filterFolder(ghc::filesystem::path* from, ghc::filesystem::path* to, FolderType folderType, RPGMakerVersion version, Platform platform);
bool filterFile(ghc::filesystem::path* from, ghc::filesystem::path* to, FolderType folderType, RPGMakerVersion version, Platform platform);
bool shouldEncryptFile(ghc::filesystem::path *from, bool encryptAudio, bool encryptImages, RPGMakerVersion version);

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
            return 0;
        }
        
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
            encryptionKey = std::string("");

        platformNames = result["platforms"].as<std::vector<std::string>>();
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
            for (const auto& p : ghc::filesystem::recursive_directory_iterator(templateFolderPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
                auto path = p.path();
                auto sPath = std::string(path.c_str());
                auto entryOutputPath = ghc::filesystem::path(platformOutputPath).append(sPath.substr(sTemplateFolderPath.length()+1));

                if (p.is_directory(ec)) {
                    if (!ensureDirectory(entryOutputPath, errorLogger))
                        return 1;
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

        //Actors.json
        //img/sv_actors/{}.png
        std::set<std::string_view> actorBattlerNames;

        //Animations.json
        //img/animations/{}.png
        std::set<std::string_view> animationNames;

        //Enemies.json
        //img/sv_enemies/{}.png
        std::set<std::string_view> enemyBattlerNames;

        //Tilesets.json
        //img/tilesets/{}.png+{}.txt
        std::set<std::string_view> tilesetNames;

        //System.json
        //img/titles1/{}.png
        std::set<std::string_view> title1Names;
        //img/titles2/{}.png
        std::set<std::string_view> title2Names;

        //other
        //img/characters/{}.png
        std::set<std::string_view> characterNames;
        //img/faces/{}.png
        std::set<std::string_view> faceNames;

        //audio/bgm/{}.m4a/ogg
        std::set<std::string_view> bgmNames;
        //audio/bgs/{}.m4a/ogg
        std::set<std::string_view> bgsNames;
        //audio/me/{}.m4a/ogg
        std::set<std::string_view> meNames;
        //audio/se/{}.m4a/ogg
        std::set<std::string_view> seNames;

        //img/pictures/{}.png
        std::set<std::string_view> pictureNames;
        //movies/{}.webm
        std::set<std::string_view> movieNames;

        //img/battlebacks1/{}.png
        std::set<std::string_view> battleback1Names;
        //img/battlebacks2/{}.png
        std::set<std::string_view> battleback2Names;
        //img/parallaxes/{}.png
        std::set<std::string_view> parallaxNames;

        auto sInputPath = std::string(inputPath.c_str());
        for (const auto& p : ghc::filesystem::recursive_directory_iterator(inputPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
            auto path = p.path();
            auto sPath = std::string(path.c_str());
            auto entryOutputPath = ghc::filesystem::path(wwwPath).append(sPath.substr(sInputPath.length()+1));

            if (p.is_directory(ec)) {
                if (!ensureDirectory(entryOutputPath, errorLogger))
                    return 1;
            } else if (p.is_regular_file(ec)) {
                auto filename = path.filename();

                if (filename.extension() == ".json" && excludeUnused) {
                    using namespace simdjson;
                    if (filename == "Actors.json") {
                        logger->info("Parsing Actors.json");
                        dom::parser parser;
                        dom::element elements = parser.load(path);

                        for (dom::element element : elements) {
                            if (element.type() == dom::element_type::NULL_VALUE) continue;
                            dom::object obj = element;
                            /*
                             * Actors.json:
                             * battlerName => img/sv_actors/{battlerName}.png
                             * characterName (w/ index) => img/characters/{characterName}.png
                             * faceName (w/ index) => img/faces/{faceName}.png
                             */

                            std::string_view battlerName = obj["battlerName"];
                            std::string_view characterName = obj["characterName"];
                            std::string_view faceName = obj["faceName"];

                            actorBattlerNames.insert(battlerName);
                            characterNames.insert(characterName);
                            faceNames.insert(faceName);
                        }
                    } else if (filename == "Animations.json") {
                        logger->info("Parsing Animations.json");
                        dom::parser parser;
                        dom::element elements = parser.load(path);

                        for (dom::element element : elements) {
                            if (element.type() == dom::element_type::NULL_VALUE) continue;
                            dom::object obj = element;
                            /*
                             * Animations.json:
                             * animation1Name => img/animations/{animation1Name}.png
                             * animation2Name => img/animations/{animation2Name}.png
                             * timings[i].se.name => audio/se/{name}.m4a/ogg
                             */

                            std::string_view animation1Name = obj["animation1Name"];
                            std::string_view animation2Name = obj["animation2Name"];

                            animationNames.insert(animation1Name);
                            animationNames.insert(animation2Name);

                            dom::element timings = obj["timings"];
                            for (dom::object timing : timings) {
                                dom::element seElement = timing["se"];
                                if (seElement.type() == dom::element_type::NULL_VALUE) continue;
                                dom::object se = seElement;

                                std::string_view seName = se["name"];

                                seNames.insert(seName);
                            }
                        }
                    } else if (filename == "CommonEvents.json") {
                        logger->info("Parsing CommonEvents.json");
                        dom::parser parser;
                        dom::element elements = parser.load(path);

                        for (dom::element element : elements) {
                            if (element.type() == dom::element_type::NULL_VALUE) continue;
                            dom::object obj = element;

                            dom::array list = obj["list"];
                            for (dom::element listItem : list) {
                                uint64_t code = listItem["code"];
                                dom::array parameters = listItem["parameters"];

                                /*
                                 * RPG Maker MV code list (only important ones):
                                 * - 101: show text with actor face, [0] is face name
                                 *
                                 * - 132: change battle bgm, [0].name is bgm name
                                 * - 133: change victory me, [0].name is me name
                                 * - 139: change defeat me, [0].name is me name
                                 * - 140: change vehicle bgm, [1].name is bgm name
                                 *
                                 * - 205: set movement route (see 505)
                                 *
                                 * - 212: show animation, [2] is index of animation (maybe not needed)
                                 * - 231: show picture, [1] is picture name
                                 *
                                 * - 241: play bgm, [0].name is bgm name
                                 * - 245: play bgs, [0].name is bgs name
                                 * - 249: play me, [0].name is me name
                                 * - 250: play se, [0].name is se name
                                 * - 261: play movie, [0] is movie name
                                 *
                                 * - 282: change tileset, [0] is index of tileset (maybe not needed)
                                 * - 283: change battle back, [0] is battlebacks1 and [1] is battlebacks2
                                 * - 284: change parallax, [0] is image name
                                 *
                                 * - 322: change actor images, [1] is face name, [3] is character name, [5] is battler name
                                 * - 323: change vehicle image, [1] is face name
                                 *
                                 * - 337: show battle animation, [1] is index of animation (maybe not needed)
                                 *
                                 * - 505: argument of 205 where each item in parameters also has code+parameters fields:
                                 *      - 41: change character image, [0] is character name
                                 *      - 44: play se, [0].name is se name
                                 */

                                if (code == 101) {
                                    std::string_view faceName = parameters.at(0);
                                    faceNames.insert(faceName);
                                } else if (code == 132 || code == 133 || code == 139 || code == 241
                                        || code == 245 || code == 249 || code == 250) {
                                    dom::object audioObj = parameters.at(0);
                                    std::string_view audioName = audioObj["name"];
                                    if (code == 132 || code == 241) {
                                        bgmNames.insert(audioName);
                                    } else if (code == 133 || code == 139 || code == 249) {
                                        meNames.insert(audioName);
                                    } else if (code == 245) {
                                        bgsNames.insert(audioName);
                                    } else if (code == 250) {
                                        seNames.insert(audioName);
                                    }
                                } else if (code == 140) {
                                    dom::object vehicleObj = parameters.at(1);
                                    std::string_view bgmName = vehicleObj["name"];
                                    bgmNames.insert(bgmName);
                                } else if (code == 231) {
                                    std::string_view pictureName = parameters.at(1);
                                    pictureNames.insert(pictureName);
                                } else if (code == 261) {
                                    std::string_view movieName = parameters.at(0);
                                    movieNames.insert(movieName);
                                } else if (code == 283) {
                                    std::string_view battlebacks1Name = parameters.at(0);
                                    std::string_view battlebacks2Name = parameters.at(1);

                                    if (!battlebacks1Name.empty())
                                        battleback1Names.insert(battlebacks1Name);
                                    if (!battlebacks2Name.empty())
                                        battleback2Names.insert(battlebacks2Name);
                                } else if (code == 284) {
                                    std::string_view parallaxName = parameters.at(0);
                                    parallaxNames.insert(parallaxName);
                                } else if (code == 322) {
                                    std::string_view faceName = parameters.at(1);
                                    std::string_view characterName = parameters.at(3);
                                    std::string_view actorBattlerName = parameters.at(5);

                                    faceNames.insert(faceName);
                                    characterNames.insert(characterName);
                                    actorBattlerNames.insert(actorBattlerName);
                                } else if (code == 323) {
                                    std::string_view faceName = parameters.at(1);
                                    faceNames.insert(faceName);
                                } else if (code == 505) {
                                    dom::object extraObj = parameters.at(0);

                                    uint64_t extraCode = extraObj["code"];
                                    dom::array extraParameters = extraObj["parameters"];

                                    if (extraCode == 41) {
                                        std::string_view characterName = extraParameters.at(0);
                                        characterNames.insert(characterName);
                                    } else if (extraCode == 44) {
                                        dom::object seObj = extraParameters.at(0);
                                        std::string_view seName = seObj["name"];
                                        seNames.insert(seName);
                                    }
                                }
                            }
                        }
                    } else if (filename == "Enemies.json") {
                        logger->info("Parsing Enemies.json");
                        dom::parser parser;
                        dom::element elements = parser.load(path);

                        for (dom::element element : elements) {
                            if (element.type() == dom::element_type::NULL_VALUE) continue;
                            dom::object obj = element;
                            /*
                             * Enemies.json:
                             * battlerName => img/sv_enemies/{battlerName}.png
                             */

                            std::string_view battlerName = obj["battlerName"];

                            enemyBattlerNames.insert(battlerName);
                        }
                    } else  if (filename == "System.json") {
                        logger->info("Parsing System.json");
                        dom::parser parser;
                        dom::object system = parser.load(path);

                        /*
                         * System.json:
                         *
                         * airship.bgm.name => audio/bgm/{}.m4a/ogg
                         * airship.characterName => img/characters/{}.png
                         *
                         * boat.bgm.name => audio/bgm/{}.m4a/ogg
                         * boat.characterName => img/characters/{}.png
                         *
                         * ship.bgm.name => audio/bgm/{}.m4a/ogg
                         * ship.characterName => img/character/{}.png
                         *
                         * battleback1Name => img/battlebacks1/{}.png
                         * battleback2Name => img/battlebacks2/{}.png
                         *
                         * battlerName => img/sv_enemy/{}.png
                         *
                         * title1Name => img/titles1/{}.png
                         * title2Name => img/titles2/{}.png
                         *
                         * sounds[n].name => audio/se/{}.m4a/ogg
                         *
                         * battleBgm.name => audio/bgm/{}.m4a/ogg
                         * titleBgm.name => audio/bgm/{}.m4a/ogg
                         *
                         * defeatMe.name => audio/me/{}.m4a/ogg
                         * gameoverMe.name => audio/me/{}.m4a/ogg
                         * victoryMe.name => audio/me/{}.m4a/ogg
                         *
                         */

                        std::string_view airshipBgm = system["airship"]["bgm"]["name"];
                        std::string_view boatBgm = system["boat"]["bgm"]["name"];
                        std::string_view shipBgm = system["ship"]["bgm"]["name"];

                        bgmNames.insert(airshipBgm);
                        bgmNames.insert(boatBgm);
                        bgmNames.insert(shipBgm);

                        std::string_view airshipCharacter = system["airship"]["characterName"];
                        std::string_view boatCharacter = system["boat"]["characterName"];
                        std::string_view shipCharacter = system["ship"]["characterName"];

                        characterNames.insert(airshipCharacter);
                        characterNames.insert(boatCharacter);
                        characterNames.insert(shipCharacter);

                        std::string_view battleback1Name = system["battleback1Name"];
                        std::string_view battleback2Name = system["battleback2Name"];

                        battleback1Names.insert(battleback1Name);
                        battleback2Names.insert(battleback2Name);

                        std::string_view battlerName = system["battlerName"];

                        enemyBattlerNames.insert(battlerName);

                        std::string_view title1Name = system["title1Name"];
                        std::string_view title2Name = system["title2Name"];

                        title1Names.insert(title1Name);
                        title2Names.insert(title2Name);

                        std::string_view battleBgm = system["battleBgm"]["name"];
                        std::string_view titleBgm = system["titleBgm"]["name"];

                        bgmNames.insert(battleBgm);
                        bgmNames.insert(titleBgm);

                        std::string_view defeatMe = system["defeatMe"]["name"];
                        std::string_view gameoverMe = system["gameoverMe"]["name"];
                        std::string_view victoryMe = system["victoryMe"]["name"];

                        meNames.insert(defeatMe);
                        meNames.insert(gameoverMe);
                        meNames.insert(victoryMe);

                        dom::array sounds = system["sounds"];
                        for (dom::object sound : sounds) {
                            std::string_view soundName = sound["name"];
                            seNames.insert(soundName);
                        }
                    } else if (filename == "Tilesets.json") {
                        logger->info("Parsing Tilesets.json");
                        dom::parser parser;
                        dom::element elements = parser.load(path);

                        for (dom::element element : elements) {
                            if (element.type() == dom::element_type::NULL_VALUE) continue;
                            dom::object obj = element;
                            /*
                             * Tilesets.json:
                             * tilesetNames[n] => img/tilesets/{}.png+{}.txt
                             */

                            dom::array names = obj["tilesetNames"];
                            for (std::string_view tilesetName : names) {
                                if (tilesetName.empty()) continue;
                                tilesetNames.insert(tilesetName);
                            }
                        }
                    } else {
                        std::string sFileName = filename;
                        //11 chars: MapXYZ.json
                        if (sFileName.length() == 11) {
                            auto res = sFileName.find("Map");
                            if (res != std::string::npos) {
                                //TODO:Map*.json
                            }
                        }
                    }
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
                        if (!updateSystemJson(path, entryOutputPath, encryptAudio, encryptImages, encryptionHash, logger, errorLogger))
                            return 1;
                        continue;
                    }
                }

                operations.emplace_back(operation);
            }
        }

        std::atomic<unsigned int> succeeded;
        std::atomic<unsigned int> failed;

        tf::Taskflow taskflow;

        taskflow.for_each(operations.begin(), operations.end(), [&](struct Operation& operation) {
            auto canUseHardlinks = operation.folderType == FolderType::Project
                ? canUseHardlinksInputToOutput
                : canUseHardlinksRPGMakerToOutput;
            if (operation.type == OperationType::Copy) {
                if(copyFile(operation.from, operation.to, canUseHardlinks, logger, errorLogger)) {
                    succeeded++;
                } else {
                    failed++;
                }
            } else if (operation.type == OperationType::Encrypt) {
                if(encryptFile(operation.from, operation.to, hash, useCache, canUseHardlinks, platform, rpgmakerVersion, logger, errorLogger)) {
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
            return 1;
        }
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

bool shouldEncryptFile(ghc::filesystem::path *from, bool encryptAudio, bool encryptImages, RPGMakerVersion version) {
    auto filename = from->filename();
    auto extension = from->extension();
    auto parent = from->parent_path();
    auto parentName = parent.filename();
    auto grandparent = parent.parent_path();
    auto grandparentName = grandparent.filename();

    if (extension != ".png" && extension != ".ogg" && extension != ".m4a")
        return false;

    if (encryptAudio && (extension == ".ogg" || extension == ".m4a")) {
        return true;
    }

    if (encryptImages && extension == ".png") {
        if (version == RPGMakerVersion::MZ) {
            //effects/Textures is not encrypted in MZ
            if (parentName == "Texture" && grandparentName == "effects")
                return false;

            //img/system gets encrypted, in MV some are left out
            if (parentName == "system" && grandparentName == "img")
                return true;
        } else if (version == RPGMakerVersion::MV) {
            if (parentName == "system" && grandparentName == "img") {
                //these are for some reason not encrypted in MV
                if (filename == "Loading.png") return false;
                if (filename == "Window.png") return false;
            }
        }

        //game icon for the window
        if (filename == "icon.png" && parentName == "icon")
            return false;

        return true;
    }

    return false;
}
