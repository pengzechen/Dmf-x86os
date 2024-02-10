#include "cfg.h"

.global _start, first
.extern init

_start:
    call init

    mov $KERNEL_DATA_SEG, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %gs
    mov %ax, %fs
    mov $0x7c00, %esp  # set stack pointer

    mov $TASK_0_TSS, %ax
    ltr %ax

    
    push $APP_DATA_SEG             # data segment
    push $task_0_stack_dpl3 + 4096 # stack pointer
    push $0x202                    # eflags
    push $APP_CODE_SEG             # code segment
    push $to_dpl_3
    iret

to_dpl_3:
    mov $APP_DATA_SEG, %ax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %ss
    mov %eax, %gs
    mov %eax, %fs
    jmp task_0_entry