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

    template<typename U>
    explicit TupleElement(U&& value) : m_value(stl::forward<U>(value)) {}

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

    template<typename U, typename... Us>
    TupleImpl(U&& value, Us&&... values) :
        TupleElement<I, T>(stl::forward<U>(value)),
        TupleImpl<I + 1, Ts...>(stl::forward<Us>(values)...) {}
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

namespace std
{

template<typename>
struct tuple_size;

template<typename... Ts>
struct tuple_size<stl::Tuple<Ts...>> : stl::IntegralConstant<size_t, sizeof...(Ts)> {};

template<size_t I, typename T>
struct tuple_element
{
    using type = stl::RemoveReference<decltype(stl::get<I>(stl::declval<T>()))>;
};

}