#ifndef VM_H
#define VM_H

#include "types.h"
#include "msr.h"

#define X86_CR4_VMXE   0x00002000

#define VMX_START		0
#define VMX_VMEXIT		1
#define VMX_EXIT		2
#define VMX_RESUME		3
#define VMX_VMABORT		4
#define VMX_VMSKIP		5

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

/* pin exit entry */

enum Ctrl_pin {
	PIN_EXTINT		= 1ul << 0,
	PIN_NMI			= 1ul << 3,
	PIN_VIRT_NMI		= 1ul << 5,
	PIN_PREEMPT		= 1ul << 6,
	PIN_POST_INTR		= 1ul << 7,
};

enum Ctrl_exi {
	EXI_SAVE_DBGCTLS	= 1UL << 2,
	EXI_HOST_64		= 1UL << 9,
	EXI_LOAD_PERF		= 1UL << 12,
	EXI_INTA		= 1UL << 15,
	EXI_SAVE_PAT		= 1UL << 18,
	EXI_LOAD_PAT		= 1UL << 19,
	EXI_SAVE_EFER		= 1UL << 20,
	EXI_LOAD_EFER		= 1UL << 21,
	EXI_SAVE_PREEMPT	= 1UL << 22,
};

enum Ctrl_ent {
	ENT_LOAD_DBGCTLS	= 1UL << 2,
	ENT_GUEST_64		= 1UL << 9,
	ENT_LOAD_PAT		= 1UL << 14,
	ENT_LOAD_EFER		= 1UL << 15,
};

/* pin exit entry */


enum Encoding {
	/* 16-Bit Control Fields */
	VPID			= 0x0000ul,
	/* Posted-interrupt notification vector */
	PINV			= 0x0002ul,
	/* EPTP index */
	EPTP_IDX		= 0x0004ul,

	/* 16-Bit Guest State Fields */
	GUEST_SEL_ES		= 0x0800ul,
	GUEST_SEL_CS		= 0x0802ul,
	GUEST_SEL_SS		= 0x0804ul,
	GUEST_SEL_DS		= 0x0806ul,
	GUEST_SEL_FS		= 0x0808ul,
	GUEST_SEL_GS		= 0x080aul,
	GUEST_SEL_LDTR		= 0x080cul,
	GUEST_SEL_TR		= 0x080eul,
	GUEST_INT_STATUS	= 0x0810ul,
	GUEST_PML_INDEX         = 0x0812ul,

	/* 16-Bit Host State Fields */
	HOST_SEL_ES		= 0x0c00ul,
	HOST_SEL_CS		= 0x0c02ul,
	HOST_SEL_SS		= 0x0c04ul,
	HOST_SEL_DS		= 0x0c06ul,
	HOST_SEL_FS		= 0x0c08ul,
	HOST_SEL_GS		= 0x0c0aul,
	HOST_SEL_TR		= 0x0c0cul,

	/* 64-Bit Control Fields */
	IO_BITMAP_A		= 0x2000ul,
	IO_BITMAP_B		= 0x2002ul,
	MSR_BITMAP		= 0x2004ul,
	EXIT_MSR_ST_ADDR	= 0x2006ul,
	EXIT_MSR_LD_ADDR	= 0x2008ul,
	ENTER_MSR_LD_ADDR	= 0x200aul,
	VMCS_EXEC_PTR		= 0x200cul,
	TSC_OFFSET		= 0x2010ul,
	TSC_OFFSET_HI		= 0x2011ul,
	APIC_VIRT_ADDR		= 0x2012ul,
	APIC_ACCS_ADDR		= 0x2014ul,
	POSTED_INTR_DESC_ADDR	= 0x2016ul,
	EPTP			= 0x201aul,
	EPTP_HI			= 0x201bul,
	VMREAD_BITMAP           = 0x2026ul,
	VMREAD_BITMAP_HI        = 0x2027ul,
	VMWRITE_BITMAP          = 0x2028ul,
	VMWRITE_BITMAP_HI       = 0x2029ul,
	EOI_EXIT_BITMAP0	= 0x201cul,
	EOI_EXIT_BITMAP1	= 0x201eul,
	EOI_EXIT_BITMAP2	= 0x2020ul,
	EOI_EXIT_BITMAP3	= 0x2022ul,
	PMLADDR                 = 0x200eul,
	PMLADDR_HI              = 0x200ful,


	/* 64-Bit Readonly Data Field */
	INFO_PHYS_ADDR		= 0x2400ul,

	/* 64-Bit Guest State */
	VMCS_LINK_PTR		= 0x2800ul,
	VMCS_LINK_PTR_HI	= 0x2801ul,
	GUEST_DEBUGCTL		= 0x2802ul,
	GUEST_DEBUGCTL_HI	= 0x2803ul,
	GUEST_EFER		= 0x2806ul,
	GUEST_PAT		= 0x2804ul,
	GUEST_PERF_GLOBAL_CTRL	= 0x2808ul,
	GUEST_PDPTE		= 0x280aul,

	/* 64-Bit Host State */
	HOST_PAT		= 0x2c00ul,
	HOST_EFER		= 0x2c02ul,
	HOST_PERF_GLOBAL_CTRL	= 0x2c04ul,

	/* 32-Bit Control Fields */
	PIN_CONTROLS		= 0x4000ul,
	CPU_EXEC_CTRL0		= 0x4002ul,
	EXC_BITMAP		= 0x4004ul,
	PF_ERROR_MASK		= 0x4006ul,
	PF_ERROR_MATCH		= 0x4008ul,
	CR3_TARGET_COUNT	= 0x400aul,
	EXI_CONTROLS		= 0x400cul,
	EXI_MSR_ST_CNT		= 0x400eul,
	EXI_MSR_LD_CNT		= 0x4010ul,
	ENT_CONTROLS		= 0x4012ul,
	ENT_MSR_LD_CNT		= 0x4014ul,
	ENT_INTR_INFO		= 0x4016ul,
	ENT_INTR_ERROR		= 0x4018ul,
	ENT_INST_LEN		= 0x401aul,
	TPR_THRESHOLD		= 0x401cul,
	CPU_EXEC_CTRL1		= 0x401eul,

	/* 32-Bit R/O Data Fields */
	VMX_INST_ERROR		= 0x4400ul,
	EXI_REASON		= 0x4402ul,
	EXI_INTR_INFO		= 0x4404ul,
	EXI_INTR_ERROR		= 0x4406ul,
	IDT_VECT_INFO		= 0x4408ul,
	IDT_VECT_ERROR		= 0x440aul,
	EXI_INST_LEN		= 0x440cul,
	EXI_INST_INFO		= 0x440eul,

	/* 32-Bit Guest State Fields */
	GUEST_LIMIT_ES		= 0x4800ul,
	GUEST_LIMIT_CS		= 0x4802ul,
	GUEST_LIMIT_SS		= 0x4804ul,
	GUEST_LIMIT_DS		= 0x4806ul,
	GUEST_LIMIT_FS		= 0x4808ul,
	GUEST_LIMIT_GS		= 0x480aul,
	GUEST_LIMIT_LDTR	= 0x480cul,
	GUEST_LIMIT_TR		= 0x480eul,
	GUEST_LIMIT_GDTR	= 0x4810ul,
	GUEST_LIMIT_IDTR	= 0x4812ul,
	GUEST_AR_ES		= 0x4814ul,
	GUEST_AR_CS		= 0x4816ul,
	GUEST_AR_SS		= 0x4818ul,
	GUEST_AR_DS		= 0x481aul,
	GUEST_AR_FS		= 0x481cul,
	GUEST_AR_GS		= 0x481eul,
	GUEST_AR_LDTR		= 0x4820ul,
	GUEST_AR_TR		= 0x4822ul,
	GUEST_INTR_STATE	= 0x4824ul,
	GUEST_ACTV_STATE	= 0x4826ul,
	GUEST_SMBASE		= 0x4828ul,
	GUEST_SYSENTER_CS	= 0x482aul,
	PREEMPT_TIMER_VALUE	= 0x482eul,

	/* 32-Bit Host State Fields */
	HOST_SYSENTER_CS	= 0x4c00ul,

	/* Natural-Width Control Fields */
	CR0_MASK		= 0x6000ul,
	CR4_MASK		= 0x6002ul,
	CR0_READ_SHADOW		= 0x6004ul,
	CR4_READ_SHADOW		= 0x6006ul,
	CR3_TARGET_0		= 0x6008ul,
	CR3_TARGET_1		= 0x600aul,
	CR3_TARGET_2		= 0x600cul,
	CR3_TARGET_3		= 0x600eul,

	/* Natural-Width R/O Data Fields */
	EXI_QUALIFICATION	= 0x6400ul,
	IO_RCX			= 0x6402ul,
	IO_RSI			= 0x6404ul,
	IO_RDI			= 0x6406ul,
	IO_RIP			= 0x6408ul,
	GUEST_LINEAR_ADDRESS	= 0x640aul,

	/* Natural-Width Guest State Fields */
	GUEST_CR0		= 0x6800ul,
	GUEST_CR3		= 0x6802ul,
	GUEST_CR4		= 0x6804ul,
	GUEST_BASE_ES		= 0x6806ul,
	GUEST_BASE_CS		= 0x6808ul,
	GUEST_BASE_SS		= 0x680aul,
	GUEST_BASE_DS		= 0x680cul,
	GUEST_BASE_FS		= 0x680eul,
	GUEST_BASE_GS		= 0x6810ul,
	GUEST_BASE_LDTR		= 0x6812ul,
	GUEST_BASE_TR		= 0x6814ul,
	GUEST_BASE_GDTR		= 0x6816ul,
	GUEST_BASE_IDTR		= 0x6818ul,
	GUEST_DR7		= 0x681aul,
	GUEST_RSP		= 0x681cul,
	GUEST_RIP		= 0x681eul,
	GUEST_RFLAGS		= 0x6820ul,
	GUEST_PENDING_DEBUG	= 0x6822ul,
	GUEST_SYSENTER_ESP	= 0x6824ul,
	GUEST_SYSENTER_EIP	= 0x6826ul,

	/* Natural-Width Host State Fields */
	HOST_CR0		= 0x6c00ul,
	HOST_CR3		= 0x6c02ul,
	HOST_CR4		= 0x6c04ul,
	HOST_BASE_FS		= 0x6c06ul,
	HOST_BASE_GS		= 0x6c08ul,
	HOST_BASE_TR		= 0x6c0aul,
	HOST_BASE_GDTR		= 0x6c0cul,
	HOST_BASE_IDTR		= 0x6c0eul,
	HOST_SYSENTER_ESP	= 0x6c10ul,
	HOST_SYSENTER_EIP	= 0x6c12ul,
	HOST_RSP		= 0x6c14ul,
	HOST_RIP		= 0x6c16ul
};


typedef struct _vmcs_hdr_t {
	uint32_t revision_id:31;
	uint32_t shadow_vmcs:1;
}vmcs_hdr_t;

typedef struct _vmcs_t {
	vmcs_hdr_t hdr;
	uint32_t abort; /* VMX-abort indicator */
	/* VMCS data */
	char data[0];
}vmcs_t;

// 上下文保护
struct regs {
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t cr2;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t eflags;
};

struct vmentry_failure {
	/* Did a vmlaunch or vmresume fail? */
	bool vmlaunch;
	/* Instruction mnemonic (for convenience). */
	const char *instr;
	/* Did the instruction return right away, or did we jump to HOST_RIP? */
	bool early;
	/* Contents of [re]flags after failed entry. */
	uint32_t flags;
};


extern bool is_vmx_supported();

#define X86_CR0_PE     0x00000001
#define X86_CR0_MP     0x00000002
#define X86_CR0_EM     0x00000004
#define X86_CR0_TS     0x00000008
#define X86_CR0_WP     0x00010000
#define X86_CR0_AM     0x00040000
#define X86_CR0_PG     0x80000000
#define X86_CR3_PCID_MASK 0x00000fff
#define X86_CR4_TSD    0x00000004
#define X86_CR4_DE     0x00000008
#define X86_CR4_PSE    0x00000010
#define X86_CR4_PAE    0x00000020
#define X86_CR4_MCE    0x00000040
#define X86_CR4_PGE    0x00000080
#define X86_CR4_PCE    0x00000100
#define X86_CR4_UMIP   0x00000800
#define X86_CR4_VMXE   0x00002000
#define X86_CR4_PCIDE  0x00020000
#define X86_CR4_SMEP   0x00100000
#define X86_CR4_SMAP   0x00200000
#define X86_CR4_PKE    0x00400000


#define X86_EFLAGS_CF    0x00000001
#define X86_EFLAGS_FIXED 0x00000002
#define X86_EFLAGS_PF    0x00000004
#define X86_EFLAGS_AF    0x00000010
#define X86_EFLAGS_ZF    0x00000040
#define X86_EFLAGS_SF    0x00000080
#define X86_EFLAGS_TF    0x00000100
#define X86_EFLAGS_IF    0x00000200
#define X86_EFLAGS_DF    0x00000400
#define X86_EFLAGS_OF    0x00000800
#define X86_EFLAGS_IOPL  0x00003000
#define X86_EFLAGS_NT    0x00004000
#define X86_EFLAGS_AC    0x00040000

#define ACTV_ACTIVE		0
#define ACTV_HLT		1


#define X86_EFLAGS_CF    0x00000001
#define X86_EFLAGS_ZF    0x00000040


#define VMX_ENTRY_FAILURE	(1ul << 31)
#define VMX_ENTRY_FLAGS		(X86_EFLAGS_CF | X86_EFLAGS_PF | X86_EFLAGS_AF | \
				 X86_EFLAGS_ZF | X86_EFLAGS_SF | X86_EFLAGS_OF)

enum Reason {
	VMX_EXC_NMI		= 0,
	VMX_EXTINT		= 1,
	VMX_TRIPLE_FAULT	= 2,  // 客户机发生错误
	VMX_INIT		= 3,
	VMX_SIPI		= 4,
	VMX_SMI_IO		= 5,
	VMX_SMI_OTHER		= 6,
	VMX_INTR_WINDOW		= 7,
	VMX_NMI_WINDOW		= 8,
	VMX_TASK_SWITCH		= 9,
	VMX_CPUID		= 10,
	VMX_GETSEC		= 11,
	VMX_HLT			= 12,
	VMX_INVD		= 13,
	VMX_INVLPG		= 14,
	VMX_RDPMC		= 15,
	VMX_RDTSC		= 16,
	VMX_RSM			= 17,
	VMX_VMCALL		= 18,
	VMX_VMCLEAR		= 19,
	VMX_VMLAUNCH		= 20,
	VMX_VMPTRLD		= 21,
	VMX_VMPTRST		= 22,
	VMX_VMREAD		= 23,
	VMX_VMRESUME		= 24,
	VMX_VMWRITE		= 25,
	VMX_VMXOFF		= 26,
	VMX_VMXON		= 27,
	VMX_CR			= 28,
	VMX_DR			= 29,
	VMX_IO			= 30,
	VMX_RDMSR		= 31,
	VMX_WRMSR		= 32,
	VMX_FAIL_STATE		= 33,
	VMX_FAIL_MSR		= 34,
	VMX_MWAIT		= 36,
	VMX_MTF			= 37,
	VMX_MONITOR		= 39,
	VMX_PAUSE		= 40,
	VMX_FAIL_MCHECK		= 41,
	VMX_TPR_THRESHOLD	= 43,
	VMX_APIC_ACCESS		= 44,
	VMX_EOI_INDUCED		= 45,
	VMX_GDTR_IDTR		= 46,
	VMX_LDTR_TR		= 47,
	VMX_EPT_VIOLATION	= 48,
	VMX_EPT_MISCONFIG	= 49,
	VMX_INVEPT		= 50,
	VMX_PREEMPT		= 52,
	VMX_INVVPID		= 53,
	VMX_WBINVD		= 54,
	VMX_XSETBV		= 55,
	VMX_APIC_WRITE		= 56,
	VMX_RDRAND		= 57,
	VMX_INVPCID		= 58,
	VMX_VMFUNC		= 59,
	VMX_RDSEED		= 61,
	VMX_PML_FULL		= 62,
	VMX_XSAVES		= 63,
	VMX_XRSTORS		= 64,
};


#define HYPERCALL_MASK		0xFFF
#define HYPERCALL_BIT		(1ul << 12)


#define VMX_START		0
#define VMX_VMEXIT		1
#define VMX_EXIT		2
#define VMX_RESUME		3
#define VMX_VMABORT		4
#define VMX_VMSKIP		5

#define HYPERCALL_BIT		(1ul << 12)
#define HYPERCALL_MASK		0xFFF
#define HYPERCALL_VMEXIT	0x1
#define HYPERCALL_VMABORT	0x2
#define HYPERCALL_VMSKIP	0x3

#endif // VM_H