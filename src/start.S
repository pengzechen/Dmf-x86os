
#include "cfg.h"

.global _start
.code16
.text

_start:
    mov $0, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %ss
    mov %eax, %gs
    mov %eax, %fs
    mov $0x7c00, %esp   // set stack pointer
# ------------------------------------#
# ----0x7c00 + 512(0x200) = 0x7e00----#


read_after_512:
    mov $0x7e00, %bx
    mov $0x2,    %cx
    mov $0x240,  %ax
    mov $0x80,   %dx
    int $0x13    # bios interupt
    jc read_after_512

    call detect_mem

    cli
    lgdt (gdt_desc)
    lidt (idt_desc)
    mov $1, %ax
    lmsw %ax

    jmp $KERNEL_CODE_SEG, $start_32
    
    # 510
    .org 0x1fe 
    .byte 0x55, 0xaa
# --------------------------------------- #
# --------------------------------------- #
# --------------------------------------- #
# --------------------------------------- #


.text
detect_mem:
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %edi

    movl $0, 0x1000
    xorl %ebx, %ebx
    movw $0x1004, %di
start_probe:
    movl $0xE820, %eax   
    movl $20, %ecx
    movl $0x534D4150, %edx
    int $0x15
    jnc cont
    movw $12345, 0x1000
    jmp finish_probe
cont:
    addw $20, %di
    incl 0x1000
    cmpl $0, %ebx
    jnz start_probe

finish_probe:
    pop %edi
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax
ret



.code32
.text
.global timer_int
.global task_0_entry
.global task_1_entry

gdt_desc:
    .word (256*8) - 1
    .long gdt_table

idt_desc:
    .word (256*8) - 1
    .long idt_table

# --------------------------------------- #
# --------------------------------------- #
timer_int:
    // push %ds

    pusha
    mov $0x20, %al
    outb %al, $0x20

    // mov $KERNEL_DATA_SEG, %ax
    // mov %ax, %ds

    call task_sched

    popa
    // pop %ds
    iret


start_32:
    mov $KERNEL_DATA_SEG, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %gs
    mov %ax, %fs
    mov $0x7c00, %esp  # set stack pointer
    
    call os_init
    
    # set cr3
    mov $page_dir, %eax
    mov %eax, %cr3
    # set cr4
    mov %cr4, %eax
    orl $(1 << 4), %eax
    mov %eax, %cr4
    # set cr0
    mov %cr0, %eax
    orl $(1 << 31), %eax
    mov %eax, %cr0

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