#include <cstdio>
#include <vector>
#include "test.h"

static std::vector<simo::Test*>& getTestList()
{
    static std::vector<simo::Test*> tests;
    return tests;
}

void simo::Test::registerTest(simo::Test* test)
{
    getTestList().push_back(test);
}

int main(int argc, char* argv[])
{
    for (auto test : getTestList()) {
        std::printf("Test: %s\n", test->getName());

        auto result = test->run();
        if (!result) {
            std::printf("Failed :(\n");
        }
    }

    return 0;
}