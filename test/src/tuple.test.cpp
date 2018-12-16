#include <cstdint>
#include "catch.hpp"

#include "stl/tuple.h"

stl::Tuple<int, int, int> f()
{
    return {1, 2, 1337};
}

TEST_CASE("tuple", "[tuple]") {
    const stl::Tuple<uint32_t, char> t{1337, 'a'};

    REQUIRE(stl::get<0>(t) == uint32_t(1337));
    REQUIRE(stl::get<1>(t) == 'a');

    auto [a, b, c] = f();
    REQUIRE(a == 1);
    REQUIRE(b == 2);
    REQUIRE(c == 1337);
}