#include <cstdint>
#include "catch.hpp"

#include "stl/flags.h"

enum class TestFlags : std::uint64_t
{
    Value1 = 1 << 0,
    Value2 = 1 << 1,
    Value3 = 1 << 2,
    Value4 = 1 << 3,
};

enum class TestFlags2 : std::uint8_t
{
    Value1 = 1 << 0,
    Value2 = 1 << 1,
    Value3 = 1 << 2,
    Value4 = 1 << 3,
};

TEST_CASE("heh", "[ebin]") {
    stl::Flags<TestFlags> f;

    REQUIRE(f.value() == 0);

    f |= TestFlags::Value1;
    REQUIRE(f.value() == 1);

    auto f2 = stl::makeFlags(TestFlags2::Value1, TestFlags2::Value2, TestFlags2::Value3, TestFlags2::Value4);
    REQUIRE(f2.value() == 0b1111);
}