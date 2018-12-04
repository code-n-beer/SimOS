#include <functional>
#include <vector>

namespace simo
{

class Test
{
public:
    Test(const char* name, std::function<bool()> func) :
        m_name(name), m_func(func)
    {
        registerTest(this);
    }

    bool run()
    {
        return m_func();
    }

    const char* getName() const
    {
        return m_name;
    }

private:
    const char* m_name;
    std::function<bool()> m_func;

    static void registerTest(Test*);
};

}

#define SIMO_TEST_NAME_IMPL2(x, y) x##y
#define SIMO_TEST_NAME_IMPL(x, y) SIMO_TEST_NAME_IMPL2(x, y)
#define SIMO_TEST_NAME SIMO_TEST_NAME_IMPL(test_, __LINE__)
#define SIMO_TEST(name, fn) static simo::Test SIMO_TEST_NAME (name, fn)