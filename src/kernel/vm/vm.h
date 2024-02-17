#ifndef VM_H
#define VM_H

#define APIC_BASE_MSR 0x800
#define MSR_IA32_FEATURE_CONTROL 0x3a

extern bool is_vmx_supported();

#endif // VM_H