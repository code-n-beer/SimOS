#pragma once

#include <stddef.h>
#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t length);
void* memmove(void* dest, const void* src, size_t length);
void* memset(void* dest, int value, size_t length);
