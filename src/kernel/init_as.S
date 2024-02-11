#include "cfg.h"

.global _start
.extern init, task_0_entry

_start:
    call init

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
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %gs
    mov %ax, %fs
    call task_0_entry
