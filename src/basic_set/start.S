
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
	mov $0x2, %ah		# ah: 0x2读磁盘命令
	mov $64, %al		# al: 读取的扇区数量, 必须小于128，暂设置成32KB
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

gdt_desc:
    .word (256*8) - 1
    .long gdt_table

idt_desc:
    .word (256*8) - 1
    .long idt_table


start_32:
    mov $KERNEL_DATA_SEG, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %gs
    mov %ax, %fs
    mov $0x7c00, %esp  # set stack pointer
    
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

    call os_init
    jmp $KERNEL_CODE_SEG, $0x100000


