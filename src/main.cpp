#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/fmt.h>
#include <ghc/filesystem.hpp>
#include <iostream>

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
        ("h,help", "Print usage");

    std::string input, output, rpgmaker, encryptionKey;
    bool encryptImages, encryptAudio;

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
        encryptAudio = result["encryptImages"].as<bool>();

        if (encryptImages || encryptAudio)
            encryptionKey = result["encryptionKey"].as<std::string>();
        else
            encryptionKey = std::string("");
    } catch (const cxxopts::OptionException& e) {
        errorLogger->error(e.what());
        return -1;
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

    if (ghc::filesystem::exists(outputPath)) {
        spdlog::stopwatch sw;
        logger->info("Output Folder exists, removing old files...");
        auto removed = 0;
        for (auto p : ghc::filesystem::recursive_directory_iterator(outputPath)) {
            if (!p.exists())
                continue;
            if (p.is_directory())
                continue;
            logger->debug("Removing file {}", p.path());
            if (ghc::filesystem::remove(p.path())) {
                removed++;
            } else {
                logger->warn("Unable to remove file {}", p.path());
            }
        }
        ghc::filesystem::remove(outputPath);
        logger->info("Finished removing {} file(s) in {} seconds", removed, sw);
    }

    if (!ghc::filesystem::create_directory(outputPath)) {
        errorLogger->error("Unable to create output directory!");
        return 1;
    }

    auto wwwPath = outputPath /= "www";
    if (!ensureDirectory(wwwPath, errorLogger))
        return 1;

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
    if (ghc::filesystem::exists(path)) return true;
    if (!ghc::filesystem::create_directory(path)) {
        errorLogger->error("Unable to create directory {}", path);
        return false;
    }
    return true;
}