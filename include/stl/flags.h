#pragma once

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

    constexpr Flags(TFlag v) :
        m_value{ValueType(v)} {}

    template<typename T = ValueType>
    constexpr T value() const { return T{m_value}; }

    constexpr Flags<TFlag> operator|(Flags<TFlag> t)  const { return Flags{ static_cast<ValueType>(m_value | t.m_value) }; }
    constexpr Flags<TFlag> operator|(TFlag t)         const { return (*this) | Flags(t); } 
    constexpr Flags<TFlag> operator&(Flags<TFlag> t)  const { return Flags{ m_value & t.m_value }; }
    constexpr Flags<TFlag> operator~()                const { return Flags{ ~m_value }; }
    constexpr operator bool()                         const { return m_value != 0; }

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

template<typename... TFlags>
constexpr auto makeFlags(TFlags... flags)
{
    return (Flags<TFlags>(flags) | ...);
}

}