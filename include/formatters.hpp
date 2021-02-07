#pragma once
#include <spdlog/fmt/fmt.h>
#include <ghc/filesystem.hpp>
#include <iostream>
#include "platform.hpp"
#include "rpgmakerVersion.hpp"

template <>
struct fmt::formatter<ghc::filesystem::path> {
    static constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const ghc::filesystem::path& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", p.u8string());
    }
};

template <>
struct fmt::formatter<std::error_code> {
    static constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const std::error_code& ec, FormatContext& ctx) {
        return format_to(ctx.out(), "\n[{}]({}): {}", ec.category().name(), ec.value(), ec.message());
    }
};

template <>
struct fmt::formatter<Platform> {
    static constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const Platform& p, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", PlatformNames[(int)p]);
    }
};

template <>
struct fmt::formatter<RPGMakerVersion> {
    static constexpr auto parse(format_parse_context& ctx) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const RPGMakerVersion& version, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", RPGMakerVersionNames[(int)version]);
    }
};
