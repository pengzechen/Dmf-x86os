
.globl	entry_sysenter, guest_entry
.extern regs, vm_syscall_handler

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


.extern guest_main, hypercall
guest_entry:
    call guest_main
    mov $1, %edi
    call hypercall
