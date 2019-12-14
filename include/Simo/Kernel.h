#pragma once

#include <stddef.h>
#include <stdint.h>

// support for placement new
void* operator new  (size_t, void* p) throw();
void* operator new[](size_t, void* p) throw();

void  operator delete  (void*) throw();
void  operator delete[](void*) throw();
void  operator delete  (void*, size_t) throw();
void  operator delete[](void*, size_t) throw();
