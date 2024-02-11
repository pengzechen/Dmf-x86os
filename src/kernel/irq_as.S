# --------------------------------------- #
# --------------------------------------- #
#include "cfg.h"
.global timer_int

timer_int:
    push %ds

    pusha
    mov $0x20, %al
    outb %al, $0x20

    mov $KERNEL_DATA_SEG, %ax
    mov %ax, %ds

    call task_sched

    popa
    pop %ds
    iret