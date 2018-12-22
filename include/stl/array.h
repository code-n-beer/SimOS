#pragma once

#include <stddef.h>
#include "typetraits.h"

namespace stl
{

template<typename T, size_t N>
class Array
{
public:
    Array() = default;
    ~Array() = default;

    Array(const Array<T, N>& other)
    {
        for (size_t i = 0; i < N; i++) {
            m_items[i] = other[i];
        }
    }

    Array(Array<T, N>&& other)
    {
        for (size_t i = 0; i < N; i++) {
            m_items[i] = stl::move(other[i]);
        }
    }

    size_t size() const
    {
        return N;
    }

    size_t byteSize() const
    {
        return size() * sizeof(T);
    }

    const T& operator[](int index) const
    {
        // TODO: assert(index > 0 && index < N)
        return m_items[index];
    }

    T& operator[](int index)
    {
        // TODO: assert(index > 0 && index < N)
        return m_items[index];
    }

    T& at(int index)
    {
        return m_items[index];
    }

    const T& at(int index) const
    {
        return m_items[index];
    }

    bool operator==(const Array<T, N>& other) const
    {
        for (size_t i = 0; i < N; i++) {
            if (m_items[i] != other.m_items[i]) {
                return false;
            }
        }

        return true;
    }

    Array<T, N>& operator=(const Array<T, N>& other)
    {
        if (this != &other) {
            for (size_t i = 0; i < N; i++) {
                m_items[i] = other.m_items[i];
            }
        }

        return *this;
    }

    Array<T, N>& operator=(Array<T, N>&& other)
    {
        if (this != &other) {
            for (size_t i = 0; i < N; i++) {
                m_items[i] = stl::move(other.m_items[i]);
            }
        }

        return *this;
    }

    T* data()
    {
        return &m_items[0];
    }

    const T* data() const
    {
        return &m_items[0];
    }

private:
    T m_items[N];
};

}