#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/fmt.h>
#include <ghc/filesystem.hpp>
#include <iostream>

#define RPGMPACKER_TESTING

bool isValidDirectory(std::string directory, std::string name, std::shared_ptr<spdlog::logger> errorLogger);
bool ensureDirectory(ghc::filesystem::path path, std::shared_ptr<spdlog::logger> errorLogger);

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

int main(int argc, char** argv) {
    auto logger = spdlog::stdout_color_mt("console");
    auto errorLogger = spdlog::stderr_color_mt("stderr");

    cxxopts::Options options("RPGMPacker", "RPGMaker Games Packer for Continues Deployment.");
    options.add_options()
        ("i,input", "Input folder containing the .rpgproj file", cxxopts::value<std::string>())
        ("o,output", "Output folder", cxxopts::value<std::string>())
        ("rpgmaker", "RPG Maker installation folder", cxxopts::value<std::string>())
        ("encryptImages", "Enable Image Encryption using encryptionKey", cxxopts::value<bool>()->default_value("false"))
        ("encryptAudio", "Enable Audio Encryption using encryptionKey", cxxopts::value<bool>()->default_value("false"))
        ("encryptionKey", "", cxxopts::value<std::string>())
        ("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Print usage");

    std::string input, output, rpgmaker, encryptionKey;
    bool encryptImages, encryptAudio, debug;

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
        debug = true;
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

    if (!isValidDirectory(input, "Input", errorLogger))
        return -1;
    if (!isValidDirectory(rpgmaker, "RPG Maker", errorLogger))
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
    if (ec) {
        errorLogger->error("Unable to create output directory!");
        return 1;
    }

    auto wwwPath = ghc::filesystem::path(outputPath).append("www");
    if (!ensureDirectory(wwwPath, errorLogger))
        return 1;

    logger->info("Copying files from {} to {}", inputPath, wwwPath);
    spdlog::stopwatch sw;
    for (auto p : ghc::filesystem::directory_iterator(inputPath, ghc::filesystem::directory_options::skip_permission_denied, ec)) {
        auto path = p.path();
        logger->debug("{}", path);
        if (p.is_directory(ec)) {
            auto dirname = path.filename();
            
            auto outputDirPath = ghc::filesystem::path(wwwPath).append(dirname);

            logger->debug("Copying folder from {} to {}", path, outputDirPath);
            ghc::filesystem::copy(path, outputDirPath, ghc::filesystem::copy_options::recursive |
                ghc::filesystem::copy_options::overwrite_existing, ec);
            if (ec) {
                errorLogger->error("Unable to copy folder from {} to {}! {}", path, outputDirPath, ec);
                ec.clear();
            }
        } else if (p.is_regular_file(ec)) {
            auto filename = path.filename();
            auto extension = path.extension();
            if (extension == ".rpgproject") continue;

            auto outputFilePath = ghc::filesystem::path(wwwPath).append(filename);
            
            logger->debug("Copying file from {} to {}", path, outputFilePath);
            auto result = ghc::filesystem::copy_file(path, outputFilePath, ghc::filesystem::copy_options::overwrite_existing, ec);
            if (ec) {
                errorLogger->error("Unable to copy file from {} to {}! {}", path, outputFilePath, ec);
                ec.clear();
            }
        }
    }
    logger->info("Finished copying files in {} seconds", sw);

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
