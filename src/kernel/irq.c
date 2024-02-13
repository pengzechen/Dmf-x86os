
#include "types.h"
#include "desc.h"
#include "cpu.h"

extern void timer_int();

extern idt_entry_t * idt_table;

void set_idt_entry(int vec, void *addr, uint16_t selector, int dpl)
{
    idt_entry_t *e = &idt_table[vec];
    e->offset0 = (uint32_t)addr;
    e->selector = selector;
    e->ist = 0;
    e->type = 0xE;
    e->dpl = dpl;
    e->p = 1;
    e->offset1 = (uint32_t)addr >> 16;
}

void set_idt_dpl(int vec, uint16_t dpl)
{
    idt_entry_t *e = &idt_table[vec];
    e->dpl = dpl;
}

void set_idt_sel(int vec, uint16_t sel)
{
    idt_entry_t *e = &idt_table[vec];
    e->selector = sel;
}

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

    int tmo = 1193180 / 1; 
    outb(0x36, 0x43);
    outb((uint8_t)tmo, 0x40);
    outb(tmo >> 8, 0x40);

    set_idt_entry(0x20, (void*)timer_int, KERNEL_CODE_SEG, 0);
}



