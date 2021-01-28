#pragma once
#include <spdlog/fmt/fmt.h>
#include <ghc/filesystem.hpp>
#include <iostream>
#include "platform.hpp"

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
