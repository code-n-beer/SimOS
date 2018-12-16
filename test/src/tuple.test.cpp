#include <cstdint>
#include "catch.hpp"

#include "stl/tuple.h"

TEST_CASE("tuple", "[tuple]") {
    const stl::Tuple<uint32_t, char> t{1337, 'a'};

    REQUIRE(stl::get<0>(t) == uint32_t(1337));
    REQUIRE(stl::get<1>(t) == 'a');
}