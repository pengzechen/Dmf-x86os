#include "string.h"
#include "cpu.h"
#include "vm.h"

extern int printf(const char *fmt, ...);

bool is_vmx_supported() {
	unsigned int eax, ebx, ecx, edx;
	cpuid(0x1, &eax, &ebx, &ecx, &edx);
    // STEP (1) Check if cpu support vt
    printf("ecx: %x", ecx);
    if (((ecx >> 5) & 1) != 0) {
        printf("This cpu support vt");
    } else {
        printf("This cpu don't support vt");
		return false;
    }
    // STEP (2) Check if main board support vt
    uint32_t md_check = rdmsr(MSR_IA32_FEATURE_CONTROL);
	if ((md_check & 0x5) == 0x5) {
		printf("VMX enabled and locked by BIOS");
		return true;
	} else if (md_check & 0x1) {
		printf("ERROR: VMX locked out by BIOS without enabled");
		return false;
	}
	// STEP (3) Enable VMX in MSR_IA32_FEATURE_CONTROL
	wrmsr(MSR_IA32_FEATURE_CONTROL, 0x5);

	return true;
}