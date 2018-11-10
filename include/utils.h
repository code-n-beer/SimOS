#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t length);
void* memmove(void* dest, const void* src, size_t length);
void* memset(void* dest, int value, size_t length);

namespace utils
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
        if (&other == this) {
            return *this;
        }

        for (size_t i = 0; i < N; i++) {
            m_items[i] = other.m_items[i];
        }
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

#endif