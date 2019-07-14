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

template<typename T>
class LambdaRef;

template<typename TResult, typename... TArgs>
class LambdaRef<TResult(TArgs...)>
{
public:
    LambdaRef() = default;

    template<typename TFunc>
    LambdaRef(TFunc&& func) :
        m_callFunc(&LambdaRef::doCall<TFunc>), m_callable(&func)
    {
    }

    template<typename... UArgs>
    TResult operator()(UArgs&&... args)
    {
        // TODO: what to do in case of nullptr?
        return m_callFunc(m_callable, static_cast<TArgs&&>(args)...);
    }

private:
    template<typename T>
    static TResult doCall(void* obj, TArgs... args)
    {
        auto func = static_cast<AddPointer<T>>(obj);
        return (*func)(args...);
    }

    using CallFunc = TResult (*)(void*, TArgs...);

    CallFunc m_callFunc = nullptr;
    void* m_callable = nullptr;
};

}