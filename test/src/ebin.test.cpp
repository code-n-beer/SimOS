#include "test.h"

#include <cstdio>

SIMO_TEST("heh", []() -> bool {
    std::printf("ebin\n");
    return true;
});

SIMO_TEST("fail", []() -> bool {
    return false;
});