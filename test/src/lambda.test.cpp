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