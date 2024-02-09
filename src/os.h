#ifndef OS_H
#define OS_H

#include "cfg.h"
#include "types.h"

/* --------------- gdt ----------------------------*/

// 大小为8字节
typedef struct _gdt_table_t {

    uint16_t limit_l;
    uint16_t base_l;
    uint16_t basehl_attr;
    uint16_t base_limit;

} gdt_table_t OS_ALIGN(8);

/* -------------- irq ---------------------------*/

// 大小为8字节
typedef struct _idt_table_t {

    uint16_t offset_l;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset_h;

} idt_table_t OS_ALIGN(8);


/* -------------  Page --------------------------*/


#define PDE_P    (1 << 0)
#define PDE_W    (1 << 1)
#define PDE_U    (1 << 2)
#define PDE_PS   (1 << 7)

typedef uint32_t page_table_t;
typedef uint32_t page_dir_t;


#endif  // OS_H