
#include "cfg.h"
#include "types.h"
#include "vm/vm.h"

extern int printf(const char *fmt, ...);
extern void irq_init();
extern void mem_init();
extern void task_init();
extern void virt_enable();
void init_syscall(void);

void init() {
    init_syscall();
    irq_init();
    mem_init();
    task_init();
    // virt_enable();
}