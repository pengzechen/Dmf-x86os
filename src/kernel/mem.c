
#include "types.h"
#include "desc.h"
#include "bios.h"

extern page_table_t * page_table;
extern page_dir_t * page_dir; 

#define MAP_ADDR 0x80000000
uint8_t map_test[4096] OS_ALIGN(4096) = {0x12};

void mem_test() {
    e820map_t info;
    e820map_t *et = (e820map_t *)(0x1000);
    info = *et;

    int index = MAP_ADDR >> 22;
    int offset = (MAP_ADDR >> 12) & 0x3ff;
    page_dir   [index]  =   (uint32_t)page_table|PDE_P|PDE_W|PDE_U;
    page_table [offset] =   (uint32_t)  map_test|PDE_P|PDE_W|PDE_U;
}

void mem_init () {
    mem_test();
}   