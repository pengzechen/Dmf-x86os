#include "cfg.h"
.globl	entry_sysenter, guest_entry
.extern regs, vm_syscall_handler
.extern test_no_access_page_addr
.text
entry_sysenter:

    xchg %eax, regs+0x00
    xchg %ebx, regs+0x04
    xchg %ecx, regs+0x08
    xchg %edx, regs+0x0c
    xchg %ebp, regs+0x10
    xchg %esi, regs+0x14
    xchg %edi, regs+0x18
	
    and	$0xf, %eax
	mov	%eax, %edi
	call	vm_syscall_handler

    xchg %eax, regs+0x00
    xchg %ebx, regs+0x04
    xchg %ecx, regs+0x08
    xchg %edx, regs+0x0c
    xchg %ebp, regs+0x10
    xchg %esi, regs+0x14
    xchg %edi, regs+0x18

	vmresume

.text
.extern guest_main, hypercall
guest_entry:
    # 测试1: cpuid 指令 (exit reason 10)
    #cpuid

    # 测试2: guest_main
    #call guest_main

    # 测试3: 第一次 hypercall - VMSKIP (3)，guest 继续运行
    #mov $3, %edi
    #call hypercall

    # 测试4: CR3 访问测试 - 触发 VM exit (reason 28 或 29)
    mov $0x111000, %eax   # 读取当前 CR3 到 EAX
    mov %cr3, %ebx        # 触发 CR3_STORE VM exit
    mov %eax, %cr3        # 触发 CR3_LOAD VM exit

    # 测试5: 页表测试 - 尝试写入只读页，触发 #PF (exception 14)
    mov test_no_access_page_addr, %eax
    movl $0x42, (%eax)     # 尝试写入只读页，应该触发 #PF

    # 测试6: 触发 HLT 指令，应该 VM-exit (reason 12)
    hlt

    # 测试7: hyper call = 1正常退出
    mov $1, %edi
    call hypercall

    # 如果成功到达这里，说明测试失败
    # 但实际上应该会触发 VM exit，所以不应该到达这里
    jmp .




    
