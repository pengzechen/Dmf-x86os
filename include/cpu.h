#ifndef CPU_H
#define CPU_H

#include "types.h"

#define APIC_BASE_MSR 0x800


static inline void outb(uint16_t port, uint8_t data) {
	__asm__ __volatile__("outb %[v], %[p]" ::[p]"d" (port), [v]"a" (data));
}

static inline uint8_t inb(uint16_t  port) {
	uint8_t rv;
	__asm__ __volatile__("inb %[p], %[v]" : [v]"=a" (rv) : [p]"d"(port));
	return rv;
}

static inline uint16_t inw(uint16_t  port) {
	uint16_t rv;
	__asm__ __volatile__("in %1, %0" : "=a" (rv) : "dN" (port));
	return rv;
}

static inline void ljmp (uint32_t task_tss) {
	uint32_t addr[] = {0, task_tss};
    __asm__ __volatile__( "ljmpl *(%[a])"::[a]"r"(addr) );
}

static inline void cpuid(unsigned int op, unsigned int *eax, unsigned int *ebx,
			 				unsigned int *ecx, unsigned int *edx)
{
	*eax = op;
	*ecx = 0;

	__asm__ __volatile__("cpuid"
	    : "=a" (*eax),
	      "=b" (*ebx),
	      "=c" (*ecx),
	      "=d" (*edx)
	    : "0" (*eax), "2" (*ecx)
	    : "memory");
}

static uint32_t rdmsr(unsigned reg)
{
    unsigned a, d;

    __asm__ __volatile__ ("rdmsr" : "=a"(a), "=d"(d) : "c"(APIC_BASE_MSR + reg/16));
    return a | (unsigned long long)d << 32;
}

static void wrmsr(unsigned reg, uint32_t val)
{
    __asm__ __volatile__ ("wrmsr" : : "a"(val), "d"(0), "c"(APIC_BASE_MSR + reg/16));
}


#endif // CPU_H