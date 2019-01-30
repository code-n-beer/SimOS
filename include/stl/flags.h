#pragma once

#include "typetraits.h"

namespace stl
{

template<typename TFlag>
class Flags
{
public:
    static_assert(__is_enum(TFlag));

    using ValueType = __underlying_type(TFlag);

    constexpr Flags() = default;

    constexpr Flags(const Flags<TFlag>& other) :
        m_value{other.m_value} {}

    constexpr Flags(Flags<TFlag>&& other) :
        m_value{other.m_value} {}
    
    template<typename... Ts>
    constexpr Flags(TFlag value, Ts... values) :
        m_value{static_cast<ValueType>((ValueType(value) | ... | ValueType(values)))}
    {
        static_assert(stl::AreSame<TFlag, Ts...>);
    }

    template<typename T = ValueType>
    constexpr T value() const { return T{m_value}; }

    constexpr Flags<TFlag> operator|(Flags<TFlag> t)  const { return Flags{ static_cast<ValueType>(m_value | t.m_value) }; }
    constexpr Flags<TFlag> operator|(TFlag t)         const { return (*this) | Flags(t); } 
    constexpr Flags<TFlag> operator&(Flags<TFlag> t)  const { return Flags{ m_value & t.m_value }; }
    constexpr Flags<TFlag> operator~()                const { return Flags{ ~m_value }; }
    constexpr explicit operator bool()                const { return m_value != 0; }

    constexpr Flags<TFlag>& operator|=(Flags<TFlag> t)
    {   
        m_value |= t.m_value;
        return *this;
    }
    
    constexpr Flags<TFlag>& operator&=(Flags<TFlag> t)
    {   
        m_value &= t.m_value;
        return *this;
    }

private:
    constexpr explicit Flags(ValueType v) :
        m_value{v} {}
    
    ValueType m_value{};
};

template<typename TFlag, typename T = typename Flags<TFlag>::ValueType>
constexpr T operator&(T a, const Flags<TFlag>& b)
{
    return static_cast<T>(a & b.value());
}

}

template<typename TFlag>
constexpr stl::Flags<TFlag> operator|(TFlag a, TFlag b)
{
    return stl::Flags{a} | b;
}