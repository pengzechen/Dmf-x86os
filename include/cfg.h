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


// 中断相关

#define PIC0_ICW1			0x20
#define PIC0_ICW2			0x21
#define PIC0_ICW3			0x21
#define PIC0_ICW4			0x21
#define PIC0_IMR			0x21

#define PIC1_ICW1			0xa0
#define PIC1_ICW2			0xa1
#define PIC1_ICW3			0xa1
#define PIC1_ICW4			0xa1
#define PIC1_IMR			0xa1

#define PIC_ICW1_ICW4		(1 << 0)		// 1 - 需要初始化ICW4
#define PIC_ICW1_ALWAYS_1	(1 << 4)		// 总为1的位
#define PIC_ICW4_8086	    (1 << 0)        // 8086工作模式

#define IRQ_PIC_START		0x20			// PIC中断起始号

#define GATE_P_PRESENT      (1 << 15)
#define GATE_DPL0           (0 << 13)
#define GATE_TYPE_IDT       (0xE << 8)



#endif // CFG_H