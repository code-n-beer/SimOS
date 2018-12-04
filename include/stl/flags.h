#pragma once

namespace stl
{

template<typename TFlag>
class Flags
{
public:
    static_assert(__is_enum(TFlag));

    using ValueType = __underlying_type(TFlag);

    Flags() = default;
    ~Flags() = default;
    Flags(const Flags<TFlag>&) = default;
    Flags(Flags<TFlag>&&) = default;

    Flags(TFlag v) :
        m_value(ValueType(v)) {}

    template<typename T = ValueType> T value() const { return T{m_value}; }

    Flags<TFlag> operator|(Flags<TFlag> t)  const { return { m_value | t.m_value }; }
    Flags<TFlag> operator|(TFlag t)         const { return (*this) | Flags(t); } 
    Flags<TFlag> operator&(Flags<TFlag> t)  const { return { m_value & t.m_value }; }
    Flags<TFlag> operator~()                const { return { ~m_value }; }
    operator bool()                         const { return m_value != 0; }

    Flags<TFlag>& operator|=(Flags<TFlag> t)
    {   
        m_value |= t.m_value;
        return *this;
    }
    
    Flags<TFlag>& operator&=(Flags<TFlag> t)
    {   
        m_value &= t.m_value;
        return *this;
    }

private:
    Flags(ValueType v) :
        m_value(v) {}
    
    ValueType m_value{};
};

}