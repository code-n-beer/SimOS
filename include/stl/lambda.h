#pragma once

#include "typetraits.h"

namespace stl
{

template<typename>
class Lambda;

template<typename TResult, typename... TArgs>
class Lambda<TResult(TArgs...)>
{
public:
    Lambda() = default;

    template<typename TFunc>
    Lambda(TFunc&& function)
    {
        static_assert(sizeof(TFunc) <= sizeof(m_callableStorage));
        m_callable = new (m_callableStorage) Callable<TFunc>(function);
    }

    ~Lambda()
    {
        if (m_callable) {
            m_callable->~CallableBase();
        }
    }

    template<typename... TCallArgs>
    TResult operator()(TCallArgs&&... args) const
    {
        return m_callable->call(forward<TCallArgs>(args)...);
    }

private:
    class CallableBase
    {
    public:
        virtual TResult call(TArgs... args) const = 0;
        virtual ~CallableBase() {}
    };

    template<typename TFunc>
    class Callable : public CallableBase
    {
    public:
        template<typename TFuncImpl>
        Callable(TFuncImpl&& func) :
            m_function{func}
        {}

        virtual TResult call(TArgs... args) const override
        {
            return m_function(forward<TArgs>(args)...);
        }

    private:
        TFunc m_function;
    };

    char m_callableStorage[128];
    CallableBase* m_callable = nullptr;
};

}