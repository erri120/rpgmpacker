#pragma once
#include <string>

enum class Platform {
    Windows = 0,
    OSX = 1,
    Linux = 2,
    Browser = 3,
    Mobile = 4
};

static const std::string PlatformNames[] = { std::string("Windows"), std::string("OSX"), std::string("Linux"), std::string("Browser"), std::string("Mobile") };
//mobile is not supported atm and browser does not have any template files you just dump to the www folder
static const std::string PlatformFolders[] = { std::string("nwjs-win"), std::string("nwjs-osx-unsigned"), std::string("nwjs-lnx"), std::string(""), std::string("") };
