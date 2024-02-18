
#include "cfg.h"
#include "types.h"
#include "vm/vm.h"

extern int printf(const char *fmt, ...);
extern void irq_init();
extern void mem_init();
extern void task_init();
void init_syscall(void);

void init() {
    irq_init();
    mem_init();
    task_init();
    init_syscall();
    if (!is_vmx_supported()) {
        printf("vmx is not support");
    }
}