

#include "desc.h"
#include "types.h"
#include "string.h"

extern gdt_table_t * gdt_table;

syscall_des_t syscall_des;

extern void syscall_handler();

void do_syscall (int func, char * str, char color) {
    static int row = 0;
    if (func == 2) {
        uint16_t * dest = (uint16_t*)0xb8000 + 80 * row;
        while (*str) {
            *dest++ = *str++ | (0x02 << 8);
        }
        row = (row >= 25) ? 0 : row + 1;
        for (int i = 0; i<0xfffff; i++) ;
    }
}


void init_syscall(void) {
    syscall_des.offset0 = (uint32_t)syscall_handler;
    syscall_des.offset1 = (uint32_t)syscall_handler >> 16;
    syscall_des.sellector = KERNEL_CODE_SEG;

    syscall_des.param_count = 3;
    syscall_des.type = 0b1100;
    syscall_des.dpl = 3;
    syscall_des.p = 1;

    memcpy(&gdt_table[7], &syscall_des, sizeof(syscall_des_t));
}