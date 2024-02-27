#include "cfg.h"
.globl	entry_sysenter, guest_entry
.extern regs, vm_syscall_handler
.text
entry_sysenter:

    xchg %eax, regs+0x00
    xchg %ebx, regs+0x04
    xchg %ecx, regs+0x08
    xchg %edx, regs+0x0c
    xchg %ebp, regs+0x10
    xchg %esi, regs+0x14
    xchg %edi, regs+0x18
	
    and	$0xf, %eax
	mov	%eax, %edi
	call	vm_syscall_handler

    xchg %eax, regs+0x00
    xchg %ebx, regs+0x04
    xchg %ecx, regs+0x08
    xchg %edx, regs+0x0c
    xchg %ebp, regs+0x10
    xchg %esi, regs+0x14
    xchg %edi, regs+0x18

	vmresume

.text
.extern guest_main, hypercall
guest_entry:
    # mov $2, %eax
    # push $4
    # cpuid   # 这个指令成功触发 exit reason 10
    call guest_main
    mov $1, %edi
    call hypercall


.global vmx_return1

vmx_return1:

    xchg %eax, regs+0x00
    xchg %ebx, regs+0x04
    xchg %ecx, regs+0x08
    xchg %edx, regs+0x0c
    xchg %ebp, regs+0x10
    xchg %esi, regs+0x14
    xchg %edi, regs+0x18
    
