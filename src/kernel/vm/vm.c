#include "string.h"
#include "cpu.h"
#include "vm.h"
#include "../mem/alloc.h"

extern int printf(const char *fmt, ...);

bool is_vmx_supported() {
	unsigned int eax, ebx, ecx, edx;
	cpuid(0x1, &eax, &ebx, &ecx, &edx);
    // STEP (1) Check if cpu support vt
    printf("ecx: %x", ecx);
    if ((ecx & (1 << 5)) != 0) {
        printf("This cpu support vt");
    } else {
        printf("This cpu don't support vt");
		return false;
    }
    // STEP (2) Check if main board support vt
    uint64_t md_check = rdmsr(MSR_IA32_FEATURE_CONTROL);
	if ((md_check & 0x5) == 0x5) {
		printf("VMX enabled and locked by BIOS");
		return false;
	} else if (md_check & 0x1) {
		printf("ERROR: VMX locked out by BIOS without enabled");
		return false;
	}
	// STEP (3) Enable VMX in MSR_IA32_FEATURE_CONTROL
	wrmsr(MSR_IA32_FEATURE_CONTROL, 0x5);

	return true;
}

uint64_t *vmxon_region;
void *guest_stack, *guest_syscall_stack;

union vmx_basic basic;

union vmx_ctrl_msr ctrl_pin_rev;
union vmx_ctrl_msr ctrl_cpu_rev[2];
union vmx_ctrl_msr ctrl_exit_rev;
union vmx_ctrl_msr ctrl_enter_rev;

union vmx_ept_vpid  ept_vpid;

void init_vmx () {
	uint64_t fix_cr0_set, fix_cr0_clr;
	uint64_t fix_cr4_set, fix_cr4_clr;

	// test
	printf("sizeof fix_cr0_set: %d", sizeof(fix_cr0_set));

	vmxon_region = malloc(4096);
	phys_alloc_show();
	memset(vmxon_region, 0, 4096);
	guest_stack = malloc(4096);
	phys_alloc_show();
	memset(guest_stack, 0, 4096);
	guest_syscall_stack = malloc(4096);
	phys_alloc_show();
	memset(guest_syscall_stack, 0, 4096);

	fix_cr0_set = rdmsr(MSR_IA32_VMX_CR0_FIXED0);
	fix_cr0_clr = rdmsr(MSR_IA32_VMX_CR0_FIXED1);
	fix_cr4_set = rdmsr(MSR_IA32_VMX_CR4_FIXED0);
	fix_cr4_clr = rdmsr(MSR_IA32_VMX_CR4_FIXED1);

	basic.val = rdmsr(MSR_IA32_VMX_BASIC);
	
	
	ctrl_pin_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_PIN : MSR_IA32_VMX_PINBASED_CTLS);
	ctrl_exit_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_EXIT : MSR_IA32_VMX_EXIT_CTLS);
	ctrl_enter_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_ENTRY : MSR_IA32_VMX_ENTRY_CTLS);
	ctrl_cpu_rev[0].val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_PROC : MSR_IA32_VMX_PROCBASED_CTLS);
	
	if ((ctrl_cpu_rev[0].clr & CPU_SECONDARY) != 0) {
		ctrl_cpu_rev[1].val = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
	} else {
		ctrl_cpu_rev[1].val = 0;
	}

	
	if ((ctrl_cpu_rev[1].clr & (CPU_EPT | CPU_VPID)) != 0) {
		ept_vpid.val = rdmsr(MSR_IA32_VMX_EPT_VPID_CAP);
	} else {
		ept_vpid.val = 0;
	}

	
	write_cr0((read_cr0() & fix_cr0_clr) | fix_cr0_set);
	write_cr4((read_cr4() & fix_cr4_clr) | fix_cr4_set | X86_CR4_VMXE);

	*vmxon_region = basic.revision;
}

#define X86_EFLAGS_CF    0x00000001
#define X86_EFLAGS_ZF    0x00000040


static inline int vmx_on(void)
{
	bool ret;
	uint64_t rflags = read_rflags() | X86_EFLAGS_CF | X86_EFLAGS_ZF;
	__asm__ __volatile__ ("push %1; popf; vmxon %2; setbe %0\n\t"
		      : "=q" (ret) 
			  : "q" (rflags), "m" (vmxon_region) 
			  : "cc");
	return ret;
}


void virt_enable () {
	if (!is_vmx_supported()) {
        printf("vmx is not support");
		return;
    }

	init_vmx();
	
	if (vmx_on()) {
		printf("Error: %s : vmxon failed", __func__);
		return;
	} else {
		printf("vmx on");
	}
}
