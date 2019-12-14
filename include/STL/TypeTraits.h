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
struct IntegralConstant
{
    using ValueType = T;
    using Type = IntegralConstant;

    static constexpr ValueType value = Value;
    constexpr operator ValueType() const noexcept { return value; }
    constexpr ValueType operator()() const noexcept { return value; }
};

using True = IntegralConstant<bool, true>;
using False = IntegralConstant<bool, false>;

template<bool Condition, typename T>
using EnableIf = typename detail::EnableIf<Condition, T>::Type;

template<typename T, typename U>
constexpr bool IsSame = false;

template<typename T>
constexpr bool IsSame<T, T> = true;

template<typename T, typename... Ts>
constexpr bool AreSame = (IsSame<T, Ts> && ...);

template<typename T>
constexpr bool IsEmpty = __is_empty(T);

template<typename T>
constexpr bool IsTrivial = __is_trivial(T);

template<typename T>
constexpr bool IsStandardLayout = __is_standard_layout(T);

template<typename T>
constexpr bool IsEnum = __is_enum(T);

template<typename T> constexpr bool IsLvalueReference       = false;
template<typename T> constexpr bool IsLvalueReference<T&>   = true;

template<typename T> constexpr bool IsRvalueReference       = false;
template<typename T> constexpr bool IsRvalueReference<T&&>  = true;

template<typename T> constexpr bool IsReference = IsLvalueReference<T> || IsRvalueReference<T>;

template<typename T>
using RemoveReference = typename detail::RemoveReference<T>::Type;

template<typename T>
using AddPointer = RemoveReference<T>*;

template<typename T>
constexpr bool IsPointer = false;

template<typename T>
constexpr bool IsPointer<T*> = true;

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

template<typename T>
T&& declval() noexcept;

template<typename T>
constexpr RemoveReference<T>&& move(T&& t) noexcept
{
    return static_cast<T&&>(t);
}

}
