#pragma once

#include <stdint.h>

// elf_common.h uses u_int32_t for some reason
using u_int32_t = uint32_t;

#include <sys/elf64.h>
