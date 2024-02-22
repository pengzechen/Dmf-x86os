
#include "types.h"
#include "desc.h"
#include "bios.h"
#include "alloc_page.h"
#include "alloc.h"
#include "alloc_phy.h"

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
    /* ajax-2024-2-21 0x1000 放置了一些内存信息*/
    // mem_test();

    // page_alloc_ops_enable();
    /* 暂时不需要使用页分配 */

    phys_alloc_init(0x300000, 0x7ce0000);
    phys_alloc_show();
    // void * m1 = malloc(56);
    // free(m1);
    // phys_alloc_show();

    // void * m2 = malloc(64);
    // free(m2);
    // phys_alloc_show();

    // void * m3 = malloc(90);
    // free(m3);
    // phys_alloc_show();

    // void * m4 = malloc(128);
    // free(m4);
    // phys_alloc_show();

    // void * m5 = malloc(4096);
    // free(m5);
    // phys_alloc_show();
}   