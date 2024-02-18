#ifndef VM_H
#define VM_H

#include "types.h"
#include "msr.h"

#define X86_CR4_VMXE   0x00002000


union vmx_basic {
	uint64_t val;
	struct {
		uint32_t revision;
		uint32_t	size:13,
			reserved1: 3,
			width:1,
			dual:1,
			type:4,
			insouts:1,
			ctrl:1,
			reserved2:8;
	};
};

union vmx_ctrl_msr {
	uint64_t val;
	struct {
		uint32_t set, clr;
	};
};

union vmx_ept_vpid {
	uint64_t val;
	struct {
		uint32_t:16,
			super:2,
			: 2,
			invept:1,
			: 11;
		uint32_t	invvpid:1;
	};
};

enum Ctrl0 {
	CPU_INTR_WINDOW		= 1ul << 2,
	CPU_USE_TSC_OFFSET	= 1ul << 3,
	CPU_HLT			= 1ul << 7,
	CPU_INVLPG		= 1ul << 9,
	CPU_MWAIT		= 1ul << 10,
	CPU_RDPMC		= 1ul << 11,
	CPU_RDTSC		= 1ul << 12,
	CPU_CR3_LOAD		= 1ul << 15,
	CPU_CR3_STORE		= 1ul << 16,
	CPU_CR8_LOAD		= 1ul << 19,
	CPU_CR8_STORE		= 1ul << 20,
	CPU_TPR_SHADOW		= 1ul << 21,
	CPU_NMI_WINDOW		= 1ul << 22,
	CPU_IO			= 1ul << 24,
	CPU_IO_BITMAP		= 1ul << 25,
	CPU_MSR_BITMAP		= 1ul << 28,
	CPU_MONITOR		= 1ul << 29,
	CPU_PAUSE		= 1ul << 30,
	CPU_SECONDARY		= 1ul << 31,
};

enum Ctrl1 {
	CPU_VIRT_APIC_ACCESSES	= 1ul << 0,
	CPU_EPT			= 1ul << 1,
	CPU_DESC_TABLE		= 1ul << 2,
	CPU_RDTSCP		= 1ul << 3,
	CPU_VIRT_X2APIC		= 1ul << 4,
	CPU_VPID		= 1ul << 5,
	CPU_WBINVD		= 1ul << 6,
	CPU_URG			= 1ul << 7,
	CPU_APIC_REG_VIRT	= 1ul << 8,
	CPU_VINTD		= 1ul << 9,
	CPU_RDRAND		= 1ul << 11,
	CPU_SHADOW_VMCS		= 1ul << 14,
	CPU_RDSEED		= 1ul << 16,
	CPU_PML                 = 1ul << 17,
};



extern bool is_vmx_supported();

#endif // VM_H