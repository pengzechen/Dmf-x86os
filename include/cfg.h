#ifndef CFG_H
#define CFG_H

#define KERNEL_CODE_SEG     (1 * 8)
#define KERNEL_DATA_SEG     (2 * 8)

#define APP_CODE_SEG        ((3 * 8) | 3)
#define APP_DATA_SEG        ((4 * 8) | 3)

#define TASK_0_TSS          (5 * 8)
#define TASK_1_TSS          (6 * 8)

#define OS_ALIGN(n) __attribute__((aligned(n)))

// 00009000 g     O .data	00000800 gdt_table
// 0000a000 g     O .data	00001000 page_dir
// 0000b000 g     O .bss	00000800 idt_table
// 0000c000 g     O .bss	00001000 page_table

#endif // CFG_H