#include <cstdint>
#include <cstdio>
#include "catch.hpp"

#include "stl/lambda.h"

TEST_CASE("lambda", "[lambda]") {
    int x = 5;
    stl::Lambda<void()> l1([x]() {
        printf("hehebin %d\n", x);
    });

    l1();
}

TEST_CASE("lambdaref with args", "[lambdaref]") {
    auto fn = [](int a, int b) -> int {
        return a + b;
    };

    stl::LambdaRef<int(int, int)> lr(fn);
    REQUIRE(lr(1, 1) == 2);
    REQUIRE(lr(1, 2) == 3);
}

TEST_CASE("lambdaref with state", "[lambdaref]") {
    int x = 0;
    auto fn = [&x]() {
        x = 1337;
    };

    stl::LambdaRef<void()> lr(fn);
    lr();

    REQUIRE(x == 1337);
}