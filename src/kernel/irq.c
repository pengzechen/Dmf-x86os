
#include "types.h"
#include "os.h"
#include "cpu.h"

extern void timer_int();

extern idt_table_t * idt_table;

void irq_init() {

    outb(0x11, 0x20);
    outb(0x11, 0xA0);
    outb(0x20, 0x21);

    outb(0x28, 0xA1);
    outb(1 << 2, 0x21);
    outb(2, 0xA1);
    outb(0x1, 0x21);
    outb(0x1, 0xA1);

    outb(0xfe, 0x21);  
    outb(0xff, 0xA1);  

    int tmo = 1193180 / 50; 
    outb(0x36, 0x43);
    outb((uint8_t)tmo, 0x40);
    outb(tmo >> 8, 0x40);

    idt_table[0x20].offset_l = (uint32_t)timer_int & 0xffff;
    idt_table[0x20].offset_h = (uint32_t)timer_int >> 16;
    idt_table[0x20].selector = KERNEL_CODE_SEG;
    idt_table[0x20].attr = 0x8e00;

}