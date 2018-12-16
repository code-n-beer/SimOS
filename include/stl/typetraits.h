#pragma once

namespace stl::detail
{

template<typename T> struct RemoveReference         { using Type = T; };
template<typename T> struct RemoveReference<T&>     { using Type = T; };
template<typename T> struct RemoveReference<T&&>    { using Type = T; };

template<bool, typename>    struct EnableIf             {};
template<typename T>        struct EnableIf<true, T>    { using Type = T; };

}

namespace stl
{

template<typename T, T Value>
struct Constant
{
    using ValueType = T;
    using Type = Constant;

    static constexpr ValueType value = Value;
    constexpr operator ValueType() const noexcept { return value; }
};

using True = Constant<bool, true>;
using False = Constant<bool, false>;

template<bool Condition, typename T>
using EnableIf = typename detail::EnableIf<Condition, T>::Type;

template<typename T>
using RemoveReference = typename detail::RemoveReference<T>::Type;

template<typename T>
constexpr bool IsEmpty = __is_empty(T);

template<typename T>
constexpr T&& forward(RemoveReference<T>& t) noexcept
{
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&& forward(RemoveReference<T>&& t) noexcept
{
    return static_cast<T&&>(t);
}

}