
#include "cfg.h"
#include "types.h"
#include "io.h"

extern void irq_init();
extern void mem_init();
extern void task_init();

void init() {
    irq_init();
    mem_init();
    task_init();
    k_puts((const char *)"init ok\n");
}