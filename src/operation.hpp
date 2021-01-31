#pragma once

#include <ghc/filesystem.hpp>

enum class OperationType {
    Copy,
    Encrypt
};

struct Operation
{
    ghc::filesystem::path from;
    ghc::filesystem::path to;
    OperationType type;
};
