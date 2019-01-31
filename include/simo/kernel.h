#pragma once

#include <stddef.h>
#include <stdint.h>

// support for placement new
inline void* operator new  (size_t, void* p) throw() { return p; }
inline void* operator new[](size_t, void* p) throw() { return p; }

inline void  operator delete  (void*) throw() { };
inline void  operator delete[](void*) throw() { };
inline void  operator delete  (void*, size_t) throw() { };
inline void  operator delete[](void*, size_t) throw() { };
