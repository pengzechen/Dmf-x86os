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

static inline uint64_t rdmsr(uint32_t index)
{
    uint32_t a, d;
    __asm__ __volatile__ ("rdmsr" : "=a"(a), "=d"(d) : "c"(index) : "memory");
    return a | ((uint64_t)d << 32);
}

static inline void wrmsr(uint32_t index, uint64_t val)
{
    uint32_t a = val, d = val >> 32;
    __asm__ __volatile__ ("wrmsr" : : "a"(a), "d"(d), "c"(index) : "memory");
}



static inline void write_cr0(uint32_t val)
{
    __asm__ __volatile__ ("mov %0, %%cr0" : : "r"(val) : "memory");
}

static inline uint32_t read_cr0(void)
{
    uint32_t val;
    __asm__ __volatile__ ("mov %%cr0, %0" : "=r"(val) : : "memory");
    return val;
}

static inline uint32_t read_cr3(void)
{
    uint32_t val;
    asm volatile ("mov %%cr3, %0" : "=r"(val) : : "memory");
    return val;
}

static inline void write_cr4(uint32_t val)
{
    __asm__ __volatile__ ("mov %0, %%cr4" : : "r"(val) : "memory");
}

static inline uint32_t read_cr4(void)
{
    uint32_t val;
    __asm__ __volatile__ ("mov %%cr4, %0" : "=r"(val) : : "memory");
    return val;
}



static inline unsigned long read_rflags(void)
{
	unsigned long f;
	__asm__ __volatile__ ("pushf; pop %0\n\t" : "=rm"(f));
	return f;
}


#endif // CPU_H