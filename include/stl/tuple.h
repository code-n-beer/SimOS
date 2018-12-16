#pragma once

#include <stddef.h>

#include "typetraits.h"

namespace stl::detail
{

template<size_t I, typename T>
class TupleElement
{
public:
    using ValueType = T;

    TupleElement() : m_value{} {}
    TupleElement(T&& value) : m_value(stl::forward<T>(value)) {}
    TupleElement(const TupleElement<I, T>& other) = default;

    T& get()
    {
        return m_value;
    }

    const T& get() const
    {
        return m_value;
    }

private:
    T m_value;
};

template<size_t, typename...>
class TupleImpl;

template<size_t I>
class TupleImpl<I> {};

template<size_t I, typename T, typename... Ts>
class TupleImpl<I, T, Ts...> : public TupleElement<I, T>, public TupleImpl<I + 1, Ts...>
{
public:
    using TupleElement<I, T>::TupleElement;

    TupleImpl(T&& value, Ts&&... values) :
        TupleElement<I, T>(stl::forward<T>(value)),
        TupleImpl<I + 1, Ts...>(stl::forward<Ts>(values)...) {}
};

}

namespace stl
{

template<size_t I, typename T>
typename detail::TupleElement<I, T>::ValueType& get(detail::TupleElement<I, T>& tuple)
{
    return tuple.get();
}

template<size_t I, typename T>
const typename detail::TupleElement<I, T>::ValueType& get(const detail::TupleElement<I, T>& tuple)
{
    return tuple.get();
}

template<typename... Ts>
class Tuple : public detail::TupleImpl<0, Ts...>
{
public:
    using detail::TupleImpl<0, Ts...>::TupleImpl;
};

}
