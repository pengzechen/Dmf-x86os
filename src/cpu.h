#ifndef CPU_H
#define CPU_H

#include "types.h"

static inline void outb(uint8_t data, uint16_t port) {
    __asm__ __volatile__("outb %[v], %[p]"::[p]"d"(port), [v]"a"(data));
}

#endif // CPU_H