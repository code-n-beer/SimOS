#include "catch.hpp"
#include "stl/typetraits.h"

struct TestType {};

// These should really be static_asserts but whatever
TEST_CASE("type traits", "[type-traits]") {
    REQUIRE(stl::IsEmpty<TestType> == true);

    REQUIRE(stl::IsLvalueReference<TestType&> == true);
    REQUIRE(stl::IsLvalueReference<TestType&&> == false);

    REQUIRE(stl::IsRvalueReference<TestType&> == false);
    REQUIRE(stl::IsRvalueReference<TestType&&> == true);

    REQUIRE(stl::IsReference<TestType> == false);
    REQUIRE(stl::IsReference<TestType&> == true);
    REQUIRE(stl::IsReference<TestType&&> == true);
}