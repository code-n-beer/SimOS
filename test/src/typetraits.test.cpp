#include "catch.hpp"
#include "stl/typetraits.h"

template<typename T>
struct TestTemplate {};

struct TestType {};
struct TestType2 {};

using TestTypeAlias = TestType;
using TestTypeAlias2 = TestType;

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

    REQUIRE(stl::IsSame<TestType, TestType> == true);
    REQUIRE(stl::IsSame<TestType, TestType2> == false);
    REQUIRE(stl::IsSame<TestType, TestTypeAlias> == true);

    REQUIRE(stl::AreSame<TestTypeAlias, TestType, TestTypeAlias2> == true);
    REQUIRE(stl::AreSame<int, TestType> == false);
}