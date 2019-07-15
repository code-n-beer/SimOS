#pragma once

#include <stddef.h>

#include "TypeTraits.h"

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
    [[no_unique_address]] T m_value;
};

template<size_t... Is>
class TupleIndices {};

template<typename... Ts>
using TupleIndicesFor = TupleIndices<__integer_pack(sizeof...(Ts))...>;

template<typename, typename...>
class TupleImpl;

template<size_t... Is, typename... Ts>
class TupleImpl<TupleIndices<Is...>, Ts...> : public TupleElement<Is, Ts>...
{
public:
    template<typename... Us>
    TupleImpl(Us&&... values) : TupleElement<Is, Ts>(stl::forward<Us>(values))...
    {}
};

}

namespace stl
{

template<size_t I, typename T>
detail::TupleElement<I, T>::ValueType& get(detail::TupleElement<I, T>& tuple)
{
    return tuple.get();
}

template<size_t I, typename T>
const detail::TupleElement<I, T>::ValueType& get(const detail::TupleElement<I, T>& tuple)
{
    return tuple.get();
}

template<typename... Ts>
class Tuple : public detail::TupleImpl<detail::TupleIndicesFor<Ts...>, Ts...>
{
public:
    using detail::TupleImpl<detail::TupleIndicesFor<Ts...>, Ts...>::TupleImpl;
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