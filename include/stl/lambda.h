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
        // TODO: alignment?
        static_assert(sizeof(Callable<TFunc>) <= STORAGE_SIZE);
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
        // TODO: assert(m_callable)
        return m_callable->call(forward<TCallArgs>(args)...);
    }

private:
    static constexpr size_t STORAGE_SIZE = 32;

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

    char m_callableStorage[STORAGE_SIZE];
    CallableBase* m_callable = nullptr;
};

}