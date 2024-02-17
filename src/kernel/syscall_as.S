
#include "cfg.h"
.global syscall_handler
.extern do_syscall

syscall_handler:
    push %ds
    pusha

    mov $KERNEL_DATA_SEG, %ax
    mov %ax, %ds


    mov %esp, %ebp
    push 13*4(%ebp)
    push 12*4(%ebp)
    push 11*4(%ebp)

    call do_syscall
    
    add $(3*4), %esp
    
    popa
    pop %ds

    retf $(3 * 4)