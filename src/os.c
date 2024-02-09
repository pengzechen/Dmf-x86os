
#include "os.h"
#include "types.h"
#include "cpu.h"
#include "spin_lock.h"

// 2048  size: 0x800  2k
gdt_table_t
gdt_table[256] = {

    {0x0000, 0x0000, 0x0000, 0x0000},    
    {0xffff, 0x0000, 0x9a00, 0x00cf},   
    {0xffff, 0x0000, 0x9200, 0x00cf},   

    {0xffff, 0x0000, 0xfa00, 0x00cf},    // task 0 code 24
    {0xffff, 0x0000, 0xf300, 0x00cf},    // task 0 data 32

    {0x86,        0, 0xe900,    0x0},  //task 0 tss  TASK_0_TSS
    {0x86,        0, 0xe900,    0x0},

};  

// 2048 size: 0x800  2k
idt_table_t
idt_table[256] = {0};

// 4k
page_dir_t
page_dir[1024] OS_ALIGN(4096) = {0};

// 4k
page_table_t
page_table[1024] OS_ALIGN(4096) = {0};

/*
#define MAP_ADDR 0x80000000
uint8_t map_test[4096] OS_ALIGN(4096) = {0x12};
void mem_test() {
    int index = MAP_ADDR >> 22;
    int offset = (MAP_ADDR >> 12) & 0x3ff;
    page_dir   [index]  =   (uint32_t)page_table|PDE_P|PDE_W|PDE_U;
    page_table [offset] =   (uint32_t)  map_test|PDE_P|PDE_W|PDE_U;
}
*/



uint32_t task_0_stack_dpl1[1024];
uint32_t task_0_stack_dpl3[1024];

uint32_t task_1_stack_dpl1[1024];
uint32_t task_1_stack_dpl3[1024];

void task_0_entry() {
    uint8_t color = 0xff;
    while(1) {
        color--;
    }
}

void task_1_entry() {
    uint8_t color = 0x0;
    while(1) {
        color++;
    }
}

uint32_t task0_tss[] = {
    // prelink, esp0,ss0, esp1,ss1, esp2,ss2
    0, (uint32_t)task_0_stack_dpl1+4096, KERNEL_DATA_SEG, 0x0, 0x0, 0x0, 0x0,
    // cr3, eip, eflags,  eax, ecx, edx, ebx, 
    (uint32_t)page_dir, (uint32_t)task_0_entry, 0x202, 0xa, 0xc, 0xd, 0xb,
    // esp, ebp, esi, edi,
    (uint32_t)task_0_stack_dpl3+4096, 0x1, 0x2, 0x3,
    // es, cs, ss, ds, fs, gs, ldt, iomap
    APP_DATA_SEG, APP_CODE_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, 0x0, 0x0,
};

// 26 * 4 = 104 
uint32_t task1_tss[] = {
    // prelink, esp0,ss0, esp1,ss1, esp2,ss2
    0, (uint32_t)task_1_stack_dpl1+4096, KERNEL_DATA_SEG, 0x0, 0x0, 0x0, 0x0,
    // cr3, eip, eflags,  eax, ecx, edx, ebx, 
    (uint32_t)page_dir, (uint32_t)task_1_entry, 0x202, 0xa, 0xc, 0xd, 0xb, 
    // esp, ebp, esi, edi,
    (uint32_t)task_1_stack_dpl3+4096, 0x1, 0x2, 0x3,
    // es, cs, ss, ds, fs, gs, ldt, iomap
    APP_DATA_SEG, APP_CODE_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, APP_DATA_SEG, 0x0, 0x0,
};



extern void timer_int();

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


void task_sched() {
    static int task_tss = TASK_0_TSS;
    task_tss = (task_tss == TASK_0_TSS) ? TASK_1_TSS : TASK_0_TSS;

    uint32_t addr[] = {0, task_tss};
    __asm__ __volatile__( "ljmpl *(%[a])"::[a]"r"(addr) );
}


void os_init() {
    page_dir[0] = PDE_P|PDE_W|PDE_U|PDE_PS|0;       // 4Mb
    // outb(1,2);
    irq_init();

    gdt_table[5].base_l = (uint16_t)(uint32_t)task0_tss;
    gdt_table[6].base_l = (uint16_t)(uint32_t)task1_tss;
}
