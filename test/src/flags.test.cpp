#include <cstdint>
#include "catch.hpp"

#include "stl/flags.h"

enum class TestFlags : std::uint64_t
{
    Value1 = 1 << 0,
    Value2 = 1 << 1,
    Value3 = 1 << 2,
    Value4 = 1 << 3,
    Value5 = 1UL << 32,
};

enum class TestFlags2 : std::uint8_t
{
    Value1 = 1 << 0,
    Value2 = 1 << 1,
    Value3 = 1 << 2,
    Value4 = 1 << 3,
};

static_assert(stl::Flags(TestFlags::Value5).value() == uint64_t(TestFlags::Value5));
static_assert(stl::Flags(TestFlags2::Value1, TestFlags2::Value4).value() == 0b1001);

TEST_CASE("flags", "[flags]") {
    stl::Flags<TestFlags> f;

    REQUIRE(f.value() == 0);

    f |= TestFlags::Value1;
    REQUIRE(f.value() == 1);

    auto f2 = stl::Flags(TestFlags2::Value1, TestFlags2::Value2, TestFlags2::Value3, TestFlags2::Value4);
    REQUIRE(f2.value() == 0b1111);
}