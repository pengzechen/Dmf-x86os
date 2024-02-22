
.globl	entry_sysenter, vmx_return, guest_entry
.extern regs, vm_syscall_handler

entry_sysenter:

    xchg %eax, regs+0x00
    xchg %ebx, regs+0x08
    xchg %ecx, regs+0x10
    xchg %edx, regs+0x18
    xchg %ebp, regs+0x28
    xchg %esi, regs+0x30
    xchg %edi, regs+0x38

	and	$0xf, %eax
	mov	%eax, %edi
	call	vm_syscall_handler

    xchg %eax, regs+0x00
    xchg %ebx, regs+0x08
    xchg %ecx, regs+0x10
    xchg %edx, regs+0x18
    xchg %ebp, regs+0x28
    xchg %esi, regs+0x30
    xchg %edi, regs+0x38

	vmresume

vmx_return:

    jmp .

guest_entry:

    jmp .