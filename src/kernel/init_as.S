#include "cfg.h"

.global _start
.extern init, task_0_entry, cpushow

_start:
    call init

    call cpushow

    jmp to_task_0


to_task_0:
    mov $TASK_0_TSS, %ax
    ltr %ax
/*
push [ss]
push [esp]
push [eflags]
push [cs]
push [eip]
*/
    mov $APP_DATA_SEG, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %gs
    mov %ax, %fs

    push $APP_DATA_SEG             # data segment
    push $task_0_stack_dpl3 + 4096 # stack pointer
    # push $0 // $0x202                    # eflags
    push $0x202                    # eflags
    push $APP_CODE_SEG             # code segment
    push $to_dpl_3
    iret

to_dpl_3:

    ljmp $APP_CODE_SEG, $task_0_entry


