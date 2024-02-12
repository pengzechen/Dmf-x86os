
#include "types.h"
#include "os.h"
#include "cpu.h"

extern void timer_int();

extern idt_table_t * idt_table;

void irq_init() {

    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC0_ICW2, IRQ_PIC_START);
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC0_ICW4, PIC_ICW4_8086);
    
    outb(PIC1_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    outb(PIC1_ICW3, 2);
    outb(PIC1_ICW4, PIC_ICW4_8086);
    
    outb(PIC0_IMR, 0xFE);
    outb(PIC1_IMR, 0xFF);

    int tmo = 1193180 / 100; 
    outb(0x36, 0x43);
    outb((uint8_t)tmo, 0x40);
    outb(tmo >> 8, 0x40);

    idt_table[0x20].offset_l = (uint32_t)timer_int & 0xffff;
    idt_table[0x20].offset_h = ((uint32_t)timer_int >> 16) ;//& 0xffff;
    idt_table[0x20].selector = KERNEL_CODE_SEG;
    idt_table[0x20].ist = 0;
    idt_table[0x20].p = 1;
    idt_table[0x20].type = 0xE;

}