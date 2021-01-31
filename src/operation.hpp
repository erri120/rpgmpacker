#pragma once

#include <ghc/filesystem.hpp>

#include "foldertype.hpp"

enum class OperationType {
    Copy,
    Encrypt
};

struct Operation
{
    ghc::filesystem::path from;
    ghc::filesystem::path to;
    OperationType type;
    FolderType folderType;
};
