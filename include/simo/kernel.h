#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

// support for placement new
inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

#endif