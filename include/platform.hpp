#pragma once
#include <string>

enum class Platform {
    Windows = 0,
    OSX = 1,
    Linux = 2,
    Browser = 3,
    Mobile = 4,
    None
};

static const std::string PlatformNames[] = { std::string("Windows"), std::string("OSX"), std::string("Linux"), std::string("Browser"), std::string("Mobile") };
