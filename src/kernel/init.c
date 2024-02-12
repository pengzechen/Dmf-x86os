
#include "cfg.h"
#include "types.h"

extern void irq_init();
extern void mem_init();
extern void task_init();

void init() {
    irq_init();
    mem_init();
    task_init();
}