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

protected:
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

template<size_t, typename...>
struct TypeAtIndexImpl;

template<size_t I, typename T, typename... Ts>
struct TypeAtIndexImpl<I, T, Ts...>
{
    using Type = typename TypeAtIndexImpl<I - 1, Ts...>::Type;
};

template<typename T, typename... Ts>
struct TypeAtIndexImpl<0, T, Ts...>
{
    using Type = T;
};

template<size_t I, typename... Ts>
using TypeAtIndex = typename TypeAtIndexImpl<I, Ts...>::Type;

#ifdef __clang__

template<class T, T... Is>
class IntegerSequence {};

template<size_t... Is>
using TupleIndices = IntegerSequence<size_t, Is...>;

template<typename... Ts>
using TupleIndicesFor = __make_integer_seq<IntegerSequence, size_t, sizeof...(Ts)>;

#else

template<size_t... I>
class TupleIndices {};

template<typename... Ts>
using TupleIndicesFor = TupleIndices<__integer_pack(sizeof...(Ts))...>;

#endif

template<typename, typename...>
class TupleImpl;

template<size_t... Is, typename... Ts>
class TupleImpl<TupleIndices<Is...>, Ts...> : public TupleElement<Is, Ts>...
{
public:
    template<typename... Us>
    TupleImpl(Us&&... values) : TupleElement<Is, Ts>(stl::forward<Us>(values))...
    {}

    template<size_t I, typename T = TypeAtIndex<I, Ts...>>
    typename TupleElement<I, T>::ValueType& get()
    {
        return TupleElement<I, T>::get();
    }

    template<size_t I, typename T = TypeAtIndex<I, Ts...>>
    const T& get() const
    {
        return TupleElement<I, T>::get();
    }
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
struct tuple_element;

template<size_t I, typename... Ts>
struct tuple_element<I, stl::Tuple<Ts...>>
{
    using type = stl::RemoveReference<decltype(stl::get<I>(stl::declval<stl::Tuple<Ts...>>()))>;
};

}