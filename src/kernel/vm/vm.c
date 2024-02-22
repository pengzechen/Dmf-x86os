#include "string.h"
#include "cpu.h"
#include "vm.h"
#include "../mem/alloc.h"
#include "../mem/alloc_page.h"

extern int printf(const char *fmt, ...);
extern void phys_alloc_show();

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


uint64_t * vmxon_region OS_ALIGN(4096) = (void*)0;
void * guest_stack;
void * guest_syscall_stack;

union vmx_basic basic;

union vmx_ctrl_msr ctrl_pin_rev;
union vmx_ctrl_msr ctrl_cpu_rev[2];
union vmx_ctrl_msr ctrl_exit_rev;
union vmx_ctrl_msr ctrl_enter_rev;

union vmx_ept_vpid  ept_vpid;

void init_vmx () {
	uint64_t fix_cr0_set, fix_cr0_clr;
	uint64_t fix_cr4_set, fix_cr4_clr;

	printf("sizeof fix_cr0_set: %d", sizeof(fix_cr0_set));
	printf("sizeof vmxon_region: %d", sizeof(vmxon_region));
	printf("sizeof uint8_t: %d", sizeof(uint8_t));
	printf("sizeof guest_stack: %d", sizeof(guest_stack));


	vmxon_region = alloc_page();		// 0x200000
	phys_alloc_show();
	memset(vmxon_region, 0, 4096);
	guest_stack = malloc(4096);			// 使用页分配机制，0x300000 开始
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
	
	
	// ctrl_pin_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_PIN : MSR_IA32_VMX_PINBASED_CTLS);
	// ctrl_exit_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_EXIT : MSR_IA32_VMX_EXIT_CTLS);
	// ctrl_enter_rev.val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_ENTRY : MSR_IA32_VMX_ENTRY_CTLS);
	// ctrl_cpu_rev[0].val = rdmsr(basic.ctrl ? MSR_IA32_VMX_TRUE_PROC : MSR_IA32_VMX_PROCBASED_CTLS);
	
	// if ((ctrl_cpu_rev[0].clr & CPU_SECONDARY) != 0) {
	// 	ctrl_cpu_rev[1].val = rdmsr(MSR_IA32_VMX_PROCBASED_CTLS2);
	// } else {
	// 	ctrl_cpu_rev[1].val = 0;
	// }

	// if ((ctrl_cpu_rev[1].clr & (CPU_EPT | CPU_VPID)) != 0) {
	// 	ept_vpid.val = rdmsr(MSR_IA32_VMX_EPT_VPID_CAP);
	// } else {
	// 	ept_vpid.val = 0;
	// }

	uint32_t cr0 = read_cr0();
	uint32_t cr4 = read_cr4();

	uint32_t cr0_2 = ( cr0 & fix_cr0_clr) | fix_cr0_set ;
	uint32_t cr4_2 = ( cr4 & fix_cr4_clr) | fix_cr4_set ;//| X86_CR4_VMXE ;
	
	write_cr0(cr0_2);
	write_cr4(cr4_2);
	// write_cr4(read_cr4()  | X86_CR4_VMXE);

	*(uint32_t*)(vmxon_region) = basic.revision;
}

#define X86_EFLAGS_CF    0x00000001
#define X86_EFLAGS_ZF    0x00000040


static inline bool vmx_on(void)
{
	bool ret;
	uint64_t rflags = read_rflags() | X86_EFLAGS_CF | X86_EFLAGS_ZF;
	__asm__ __volatile__ ("push %1; popf; vmxon %2; setbe %0\n\t"
		      : "=q" (ret) : "q" (rflags), "m" (vmxon_region) : "cc");
	return ret;
}

extern struct asor_guest asor_guests[];

void virt_enable () {
	struct asor_guest *guest = &asor_guests[0];

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















static void asor_guest_main(void)
{
	printf("Hello Guest\n");
}

static int asor_guest_exit_handler(void)
{
	print_vmexit_info();
	return VMX_EXIT;
}

struct asor_guest asor_guests[] = {
	{ "default guest", NULL, asor_guest_main, asor_guest_exit_handler, NULL, {0} },
	{ NULL, NULL, NULL, NULL, NULL, {0} },
};