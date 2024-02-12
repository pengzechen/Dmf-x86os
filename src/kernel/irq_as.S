# --------------------------------------- #
# --------------------------------------- #
#include "cfg.h"
.global timer_int

/*
由特权级3切换到特权级0
cpu自动压入 eip cs eflags esp ss 
共 20 字节
*/
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