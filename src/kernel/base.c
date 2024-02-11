#include "os.h"



gdt_table_t * gdt_table = (gdt_table_t *)(0x9000);
page_dir_t * page_dir = (page_dir_t *)(0xa000);
idt_table_t * idt_table = (idt_table_t *)(0xb000);
page_table_t * page_table = (page_table_t *)(0xc000);
