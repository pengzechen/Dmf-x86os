#include "string.h"
#include "cpu.h"
#include "cfg.h"
#include "desc.h"
#include "vm.h"
#include "../mem/alloc.h"
#include "../mem/alloc_page.h"

extern int printf(const char *fmt, ...);
extern void phys_alloc_show();

extern gdt_table_t * gdt_table;
extern idt_entry_t * idt_table;

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


union vmx_basic basic;
uint64_t * vmxon_region OS_ALIGN(4096) = (void*)0;
vmcs_t   * vmcs         OS_ALIGN(4096) = (void*)0;

void * guest_stack;
void * guest_syscall_stack;



void init_vmx () {
	uint64_t fix_cr0_set, fix_cr0_clr;
	uint64_t fix_cr4_set, fix_cr4_clr;

	printf("sizeof fix_cr0_set: %d", sizeof(fix_cr0_set));		// sizeof Byte
	printf("sizeof vmxon_region: %d", sizeof(vmxon_region));
	printf("sizeof guest_stack: %d", sizeof(guest_stack));

	vmxon_region = (void*)0x201000;			// 4k algin
	memset(vmxon_region, 0, 4096);

	fix_cr0_set = rdmsr(MSR_IA32_VMX_CR0_FIXED0);
	fix_cr0_clr = rdmsr(MSR_IA32_VMX_CR0_FIXED1);
	fix_cr4_set = rdmsr(MSR_IA32_VMX_CR4_FIXED0);
	fix_cr4_clr = rdmsr(MSR_IA32_VMX_CR4_FIXED1);

	basic.val = rdmsr(MSR_IA32_VMX_BASIC);

	uint32_t cr0 = read_cr0();
	uint32_t cr4 = read_cr4();

	uint32_t cr0_2 = ( cr0 & fix_cr0_clr) | fix_cr0_set ;
	uint32_t cr4_2 = ( cr4 & fix_cr4_clr) | fix_cr4_set ;//| X86_CR4_VMXE ;
	
	write_cr0(cr0_2);
	write_cr4(cr4_2);

	*(uint32_t*)(vmxon_region) = basic.revision;
}


/* ---------------------------- 内联汇编函数定义区 -------------------------------*/

#define X86_EFLAGS_CF    0x00000001
#define X86_EFLAGS_ZF    0x00000040

static inline bool vmx_on(void)
{
	bool ret;
	uint32_t rflags = read_rflags() | X86_EFLAGS_CF | X86_EFLAGS_ZF;
	__asm__ __volatile__ ("push %1; popf; vmxon %2; setbe %0\n\t"
		      : "=q" (ret) : "q" (rflags), "m" (vmxon_region) : "cc");
	return ret;
}

static inline int vmcs_clear(vmcs_t *vmcs)
{
	bool ret;
	uint32_t rflags = read_rflags() | X86_EFLAGS_CF | X86_EFLAGS_ZF;

	asm volatile ("push %1; popf; vmclear %2; setbe %0"
		      : "=q" (ret) : "q" (rflags), "m" (vmcs) : "cc");
	return ret;
}

static inline int make_vmcs_current(vmcs_t *vmcs)
{
	bool ret;
	uint32_t rflags = read_rflags() | X86_EFLAGS_CF | X86_EFLAGS_ZF;

	asm volatile ("push %1; popf; vmptrld %2; setbe %0"
		      : "=q" (ret) : "q" (rflags), "m" (vmcs) : "cc");
	return ret;
}

static inline int vmcs_write(enum Encoding enc, uint32_t val)
{
	bool ret;
	asm volatile ("vmwrite %1, %2; setbe %0"
		: "=q"(ret) : "rm" (val), "r" ((uint32_t)enc) : "cc");
	if (ret == 1) {
		printf("vmcs write wrong");
	}
	return ret;	// vmwrite指令执行失败，则CF标志位会被设置为1
				// 如果CF标志位和ZF标志位中的任意一个被设置，则将目标操作数设置为1，否则设置为0。
}

static inline uint32_t read_cr3(void)
{
    uint32_t val;
    asm volatile ("mov %%cr3, %0" : "=r"(val) : : "memory");
    return val;
}

/* ---------------------------- 内联汇编函数定义区 -------------------------------*/


int clear_ptrld () {
	vmcs = (void*)0x202000;    // 4k align
	memset(vmcs, 0, 4096);

	vmcs->hdr.revision_id = basic.revision;
	/* vmclear first to init vmcs */
	if (vmcs_clear(vmcs)) {
		printf("%s : vmcs_clear error\n", __func__);
		return 1;
	}

	if (make_vmcs_current(vmcs)) {
		printf("%s : make_vmcs_current error\n", __func__);
		return 1;
	}
}

uint32_t vpid_cnt;

union vmx_ctrl_msr ctrl_pin_rev;
union vmx_ctrl_msr ctrl_exit_rev;
union vmx_ctrl_msr ctrl_enter_rev;
union vmx_ctrl_msr ctrl_cpu_rev[2];
union vmx_ept_vpid  ept_vpid;

uint32_t ctrl_pin; 
uint32_t ctrl_exit;
uint32_t ctrl_enter;
uint32_t ctrl_cpu[2];

// 上下文保护
struct regs regs;

// nochange
static void init_vmcs_ctrl(void)
{
	/* 26.2 CHECKS ON VMX CONTROLS AND HOST-STATE AREA */
	/* 26.2.1.1 */
	vmcs_write(PIN_CONTROLS, ctrl_pin);
	/* Disable VMEXIT of IO instruction */
	vmcs_write(CPU_EXEC_CTRL0, ctrl_cpu[0]);
	if (ctrl_cpu_rev[0].set & CPU_SECONDARY) {

		ctrl_cpu[1] = (ctrl_cpu[1] | ctrl_cpu_rev[1].set) & ctrl_cpu_rev[1].clr;
		vmcs_write(CPU_EXEC_CTRL1, ctrl_cpu[1]);
	}
	vmcs_write(CR3_TARGET_COUNT, 0);
	vmcs_write(VPID, ++vpid_cnt);
}

extern void entry_sysenter();
extern void guest_entry();
extern void vmx_return();

void vm_syscall_handler (uint32_t no) {
	printf("no: %x", no);
}

static void init_vmcs_host(void)
{
	/* 26.2 CHECKS ON VMX CONTROLS AND HOST-STATE AREA */
	/* 26.2.1.2 */
	vmcs_write(HOST_EFER, rdmsr(MSR_EFER));

	/* 26.2.1.3 */
	vmcs_write(ENT_CONTROLS, ctrl_enter);
	vmcs_write(EXI_CONTROLS, ctrl_exit);

	/* 26.2.2 */
	vmcs_write(HOST_CR0, read_cr0());
	vmcs_write(HOST_CR3, read_cr3());
	vmcs_write(HOST_CR4, read_cr4());
	vmcs_write(HOST_SYSENTER_EIP, (uint32_t)(&entry_sysenter));		// change
	vmcs_write(HOST_SYSENTER_CS,  KERNEL_CODE_SEG);					// change 

	/* 26.2.3 */
	vmcs_write(HOST_SEL_CS, KERNEL_CODE_SEG);
	vmcs_write(HOST_SEL_SS, KERNEL_DATA_SEG);
	vmcs_write(HOST_SEL_DS, KERNEL_DATA_SEG);
	vmcs_write(HOST_SEL_ES, KERNEL_DATA_SEG);
	vmcs_write(HOST_SEL_FS, KERNEL_DATA_SEG);
	vmcs_write(HOST_SEL_GS, KERNEL_DATA_SEG);

	vmcs_write(HOST_SEL_TR, 0x80);  // TSS_MAIN
	// vmcs_write(HOST_BASE_TR, tss_descr.base);

	vmcs_write(HOST_BASE_GDTR, (uint32_t)gdt_table);   // gdt64_desc.base
	vmcs_write(HOST_BASE_IDTR, (uint32_t)idt_table);   // idt_descr.base
	vmcs_write(HOST_BASE_FS, 0);
	vmcs_write(HOST_BASE_GS, 0);

	/* Set other vmcs area */
	vmcs_write(PF_ERROR_MASK, 0);
	vmcs_write(PF_ERROR_MATCH, 0);
	vmcs_write(VMCS_LINK_PTR, ~0ul);
	vmcs_write(VMCS_LINK_PTR_HI, ~0ul);
	vmcs_write(HOST_RIP, (uint32_t)(&vmx_return));
}

static void init_vmcs_guest(void)
{
	/* 26.3 CHECKING AND LOADING GUEST STATE */
	uint32_t guest_cr0, guest_cr4, guest_cr3;
	/* 26.3.1.1 */
	guest_cr0 = read_cr0();
	guest_cr3 = read_cr3();
	guest_cr4 = read_cr4();

	vmcs_write(GUEST_CR0, guest_cr0);
	vmcs_write(GUEST_CR3, guest_cr3);
	vmcs_write(GUEST_CR4, guest_cr4);

	/* 26.3.1.2 */

	vmcs_write(GUEST_SEL_CS, KERNEL_CODE_SEG);
	vmcs_write(GUEST_SEL_SS, KERNEL_DATA_SEG);
	vmcs_write(GUEST_SEL_DS, KERNEL_DATA_SEG);
	vmcs_write(GUEST_SEL_ES, KERNEL_DATA_SEG);
	vmcs_write(GUEST_SEL_FS, KERNEL_DATA_SEG);
	vmcs_write(GUEST_SEL_GS, KERNEL_DATA_SEG);
	vmcs_write(GUEST_SEL_LDTR, 0);
	vmcs_write(GUEST_SEL_TR, 0x80); // TSS_MAIN

	vmcs_write(GUEST_BASE_CS, 0);
	vmcs_write(GUEST_BASE_ES, 0);
	vmcs_write(GUEST_BASE_SS, 0);
	vmcs_write(GUEST_BASE_DS, 0);
	vmcs_write(GUEST_BASE_FS, 0);
	vmcs_write(GUEST_BASE_GS, 0);
	vmcs_write(GUEST_BASE_LDTR, 0);
	// vmcs_write(GUEST_BASE_TR, tss_descr.base);


	vmcs_write(GUEST_LIMIT_CS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_DS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_ES, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_SS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_FS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_GS, 0xFFFFFFFF);
	vmcs_write(GUEST_LIMIT_LDTR, 0xffff);
	// vmcs_write(GUEST_LIMIT_TR, tss_descr.limit);

	vmcs_write(GUEST_AR_CS, 0xa09b);
	vmcs_write(GUEST_AR_DS, 0xc093);
	vmcs_write(GUEST_AR_ES, 0xc093);
	vmcs_write(GUEST_AR_FS, 0xc093);
	vmcs_write(GUEST_AR_GS, 0xc093);
	vmcs_write(GUEST_AR_SS, 0xc093);
	vmcs_write(GUEST_AR_LDTR, 0x82);
	vmcs_write(GUEST_AR_TR, 0x8b);



	vmcs_write(GUEST_SYSENTER_CS,  KERNEL_CODE_SEG);
	vmcs_write(GUEST_SYSENTER_ESP, (uint32_t)(guest_syscall_stack + PAGE_SIZE - 1));
	vmcs_write(GUEST_SYSENTER_EIP, (uint32_t)(&entry_sysenter));
	vmcs_write(GUEST_DR7, 0);
	vmcs_write(GUEST_EFER, rdmsr(MSR_EFER));

	// /* 26.3.1.3 */
	vmcs_write(GUEST_BASE_GDTR, (uint32_t)gdt_table);   // gdt64_desc.base 全局gdt
	vmcs_write(GUEST_BASE_IDTR, (uint32_t)idt_table);   // idt_descr.base  全局idt

	vmcs_write(GUEST_LIMIT_GDTR, 0x7ff);  // gdt64_desc.limit   2047
	vmcs_write(GUEST_LIMIT_IDTR, 0x7ff);  // idt_descr.limit    2047

	// /* 26.3.1.4 */
	vmcs_write(GUEST_RIP, (uint32_t)(&guest_entry));
	vmcs_write(GUEST_RSP, (uint32_t)(guest_stack + PAGE_SIZE - 1));
	vmcs_write(GUEST_RFLAGS, 0x2);

	// /* 26.3.1.5 */
	vmcs_write(GUEST_ACTV_STATE, ACTV_ACTIVE);
	vmcs_write(GUEST_INTR_STATE, 0);
}



void vmcs_init () {
		
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

	/* All settings to pin/exit/enter/cpu
	   control fields should be placed here */
	ctrl_pin |= PIN_EXTINT | PIN_NMI | PIN_VIRT_NMI;
	ctrl_exit = EXI_LOAD_EFER | EXI_HOST_64;
	ctrl_enter = (ENT_LOAD_EFER | ENT_GUEST_64);
	/* DIsable IO instruction VMEXIT now */
	ctrl_cpu[0] &= (~(CPU_IO | CPU_IO_BITMAP));
	ctrl_cpu[1] = 0;

	ctrl_pin = (ctrl_pin | ctrl_pin_rev.set) & ctrl_pin_rev.clr;
	ctrl_enter = (ctrl_enter | ctrl_enter_rev.set) & ctrl_enter_rev.clr;
	ctrl_exit = (ctrl_exit | ctrl_exit_rev.set) & ctrl_exit_rev.clr;
	ctrl_cpu[0] = (ctrl_cpu[0] | ctrl_cpu_rev[0].set) & ctrl_cpu_rev[0].clr;


	init_vmcs_ctrl();
	init_vmcs_host();

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
	}

	if (clear_ptrld() != 0) {
		printf("Error: %s : vmclear vmptrld failed", __func__);
	}

	vmcs_init();

}

