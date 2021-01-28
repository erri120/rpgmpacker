#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/fmt.h>
#include <ghc/filesystem.hpp>
#include <iostream>
#include <string>
#include <numeric>

#define RPGMPACKER_TESTING

enum class Platform {
    Windows = 0,
    OSX = 1,
    Linux = 2,
    Browser = 3,
    Mobile = 4
};

static const std::string PlatformNames[] = { std::string("Windows"), std::string("OSX"), std::string("Linux"), std::string("Browser"), std::string("Mobile") };
static const std::string PlatformFolders[] = { std::string("nwjs-win"), std::string("nwjs-osx-unsigned"), std::string("nwjs-lnx"), std::string(""), std::string("") };

bool isValidDirectory(std::string directory, std::string name, std::shared_ptr<spdlog::logger> errorLogger);
bool ensureDirectory(ghc::filesystem::path path, std::shared_ptr<spdlog::logger> errorLogger);
bool getPlatforms(std::vector<std::string>* names, std::vector<Platform>* platforms, std::shared_ptr<spdlog::logger> errorLogger);

template <>
struct fmt::formatter<ghc::filesystem::path> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const ghc::filesystem::path& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", p.c_str());
    }
};

template <>
struct fmt::formatter<std::error_code> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const std::error_code& ec, FormatContext& ctx) {
        return format_to(ctx.out(), "[{}]({}): {}", ec.category().name(), ec.value(), ec.message());
    }
};

template <>
struct fmt::formatter<Platform> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Platform& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", PlatformNames[(int)p]);
    }
};

int main(int argc, char** argv) {
    auto logger = spdlog::stdout_color_mt("console");
    auto errorLogger = spdlog::stderr_color_mt("stderr");

    cxxopts::Options options("RPGMPacker", "RPGMaker Games Packer for Continues Deployment.");
    options.add_options()
        ("i,input", "Input folder containing the .rpgproj file", cxxopts::value<std::string>())
        ("o,output", "Output folder", cxxopts::value<std::string>())
        ("rpgmaker", "RPG Maker installation folder", cxxopts::value<std::string>())
        ("p,platforms", "Platforms to build for, this can take a list of platforms delimited with a comma or just one value. Possible values: win, osx, linux, browser, mobile", cxxopts::value<std::vector<std::string>>())
        ("encryptImages", "Enable Image Encryption using encryptionKey", cxxopts::value<bool>()->default_value("false"))
        ("encryptAudio", "Enable Audio Encryption using encryptionKey", cxxopts::value<bool>()->default_value("false"))
        ("encryptionKey", "", cxxopts::value<std::string>())
        ("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");

    std::string input, output, rpgmaker, encryptionKey;
    bool encryptImages, encryptAudio, debug;
    std::vector<std::string> platformNames;

    try {
        auto result = options.parse(argc, argv);
        
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

#ifdef RPGMPACKER_TESTING
        input = "E:\\Projects\\RPGMakerTest\\src\\Project1";
        output = "E:\\Projects\\RPGMakerTest\\out-c";
        rpgmaker = "M:\\SteamLibrary\\steamapps\\common\\RPG Maker MV";
        encryptImages = false;
        encryptAudio = false;
        encryptionKey = std::string("");
        //debug = true;
        debug = false;

        platformNames = std::vector<std::string>();
        platformNames.emplace_back("win");
        platformNames.emplace_back("osx");
        platformNames.emplace_back("linux");
        platformNames.emplace_back("browser");
#else
    input = result["input"].as<std::string>();
    output = result["output"].as<std::string>();
    rpgmaker = result["rpgmaker"].as<std::string>();
    encryptImages = result["encryptImages"].as<bool>();
    encryptAudio = result["encryptImages"].as<bool>();
    debug = result["debug"].as<bool>();

    if (encryptImages || encryptAudio)
        encryptionKey = result["encryptionKey"].as<std::string>();
    else
        encryptionKey = std::string("");

    platformNames = result["platforms"].as<std::vector<std::string>>();
#endif
    } catch (const cxxopts::OptionException& e) {
        errorLogger->error(e.what());
        return -1;
    }

    if (debug) {
        logger->set_level(spdlog::level::debug);
        errorLogger->set_level(spdlog::level::debug);
    }

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

                    logger->debug("Copying folder from {} to {}", path, outputDirPath);
                    ghc::filesystem::copy(path, outputDirPath, ghc::filesystem::copy_options::recursive |
                        ghc::filesystem::copy_options::overwrite_existing, ec);
                    if (ec) {
                        errorLogger->error("Unable to copy folder from {} to {}! {}", path, outputDirPath, ec);
                        ec.clear();
                    }
                } else if (p.is_regular_file(ec)) {
                    auto filename = path.filename();
                    auto outputFilePath = ghc::filesystem::path(platformOutputPath).append(filename);
                    
                    logger->debug("Copying file from {} to {}", path, outputFilePath);
                    auto result = ghc::filesystem::copy_file(path, outputFilePath, ghc::filesystem::copy_options::overwrite_existing, ec);
                    if (ec) {
                        errorLogger->error("Unable to copy file from {} to {}! {}", path, outputFilePath, ec);
                        ec.clear();
                    }
                }
            }
        }

        auto wwwPath = platform == Platform::OSX 
            ? ghc::filesystem::path(platformOutputPath).append("Game.app/Contents/Resources/app.nw") 
            : ghc::filesystem::path(platformOutputPath).append("www");
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

                    logger->debug("Copying file from {} to {}", filepath, outputFilePath);
                    auto result = ghc::filesystem::copy_file(filepath, outputFilePath, ghc::filesystem::copy_options::overwrite_existing, ec);
                    if (ec) {
                        errorLogger->error("Unable to copy file from {} to {}! {}", filepath, outputFilePath, ec);
                        return 1;
                    }
                }
            } else if (p.is_regular_file(ec)) {
                auto filename = path.filename();
                auto extension = path.extension();
                //don't ship project files, maybe also skip package.json
                if (extension == ".rpgproject") continue;

                auto outputFilePath = ghc::filesystem::path(wwwPath).append(filename);
                
                logger->debug("Copying file from {} to {}", path, outputFilePath);
                auto result = ghc::filesystem::copy_file(path, outputFilePath, ghc::filesystem::copy_options::overwrite_existing, ec);
                if (ec) {
                    errorLogger->error("Unable to copy file from {} to {}! {}", path, outputFilePath, ec);
                    return 1;
                }
            }
        }
        logger->info("Finished copying files for {} in {} seconds", platform, sw);
    }

    return 0;
}

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
