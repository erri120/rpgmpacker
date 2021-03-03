#include <vector>

#include <doctest/doctest.h>

#include <md5.h>
#include <utility.hpp>

TEST_CASE("Test MD5 Hash") {
    MD5 md5;
    auto original = "1337";
    auto encryptionHash = md5(original);
    auto hash = stringToHexHash(encryptionHash);

    CHECK_EQ(encryptionHash, "e48e13207341b6bffb7fb1622282247b");

    static const std::vector<unsigned int> hex = {228,142,19,32,115,65,182,191,251,127,177,98,34,130,36,123};

    for (auto i = 0; i < 16; i++) {
        auto a = hash[i];
        auto b = hex[i];

        CHECK_EQ(a, b);
    }
}