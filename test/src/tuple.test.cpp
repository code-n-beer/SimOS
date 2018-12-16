#include <cstdint>
#include "catch.hpp"

#include "stl/tuple.h"

TEST_CASE("tuple", "[tuple]") {
    stl::Tuple<uint32_t, char> t{1337, 'a'};

    auto v = stl::get<0, int>(t);

}