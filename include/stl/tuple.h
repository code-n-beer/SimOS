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

}

namespace stl
{

template<size_t... Is>
struct IndexSequence : Constant<size_t, Is>... {};

template<typename... Ts>
using TypeIndexSequence = IndexSequence<__integer_pack(sizeof...(Ts))...>;

template<typename... Ts>
class Tuple : public detail::TupleElement<TypeIndexSequence<Ts>::value, Ts>...
{
public:
    Tuple(Ts&&... values) :
        detail::TupleElement<TypeIndexSequence<Ts>::value, Ts>(stl::forward<Ts>(values))...
    {}
};

template<size_t I, typename T, typename... Ts>
typename detail::TupleElement<I, T>::ValueType& get(Tuple<Ts...>& tuple)
{
    return tuple.detail::TupleElement<I, T>::get();
}

}