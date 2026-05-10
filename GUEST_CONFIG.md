# Guest 配置与测试说明

本文件说明 Intel VMX Guest 的 VMCS 配置细节，以及 `guest_entry`（vm_as.S）中各测试片段的工作原理。

---

## 一、VMX 启动流程概览

```
virt_enable()
  ├─ is_vmx_supported()   检查 CPUID + IA32_FEATURE_CONTROL MSR
  ├─ init_vmx()           调整 CR0/CR4，写 VMXON region revision_id
  ├─ vmx_on()             VMXON 指令 → 进入 VMX root 模式
  ├─ clear_ptrld()        VMCLEAR + VMPTRLD → 激活 VMCS
  ├─ vmcs_init()          配置所有 VMCS 字段
  └─ vmx_run()            VMLAUNCH / VMRESUME 循环
```

---

## 二、VMCS Controls 配置

### 2.1 PIN-based Controls

```c
ctrl_pin |= PIN_NMI | PIN_VIRT_NMI;
// 最终值再与 capability MSR 做 & 掩码
```

| 位          | 含义                     | 效果                                |
|-------------|--------------------------|-------------------------------------|
| `PIN_NMI`   | NMI 触发 VM exit         | guest 收到 NMI 时退出               |
| `PIN_VIRT_NMI` | 虚拟 NMI               | 配合 NMI window 使用                |
| 外部中断位  | **未设置**               | host 的定时器中断不打断 guest       |

### 2.2 CPU-based Controls (Primary)

```c
ctrl_cpu[0] &= ~(CPU_IO | CPU_IO_BITMAP);    // 禁用 IO 指令 VM exit
ctrl_cpu[0] |= CPU_CR3_LOAD | CPU_CR3_STORE; // 开启 CR3 读写拦截
ctrl_cpu[0] |= CPU_HLT;                       // 开启 HLT 拦截
```

| 控制位           | 状态    | 说明                         |
|-----------------|---------|------------------------------|
| `CPU_IO`        | **关闭** | guest 可自由执行 IN/OUT     |
| `CPU_CR3_LOAD`  | **开启** | `MOV CR3, reg` → VM exit   |
| `CPU_CR3_STORE` | **开启** | `MOV reg, CR3` → VM exit   |
| `CPU_HLT`       | **开启** | `HLT` 指令 → VM exit       |

### 2.3 Exception Bitmap

```c
vmcs_write(EXC_BITMAP, 0xFFFFFFFF);  // 所有异常向量都触发 VM exit
```

所有 32 个异常向量（#DE, #PF, #GP, …）均设为拦截，这样 guest 发生任何异常都会退回 host 处理。

### 2.4 Exit / Entry Controls

```c
ctrl_exit  = 0;  // 无额外 exit 控制（32位模式不需要 EXI_HOST_64）
ctrl_enter = 0;  // 无额外 entry 控制（32位 guest，不设 ENT_GUEST_64）
```

两者均交由 capability MSR 强制设置必要位，不主动开启 EFER 保存/恢复等 64 位特性。

---

## 三、VMCS Host State（VM exit 后的返回环境）

Host state 描述 VM exit 后 CPU 恢复的寄存器状态，即"回到 host 内核的哪个状态"。

| VMCS 字段        | 值                            | 说明                          |
|-----------------|-------------------------------|-------------------------------|
| `HOST_CR0`      | 当前 host CR0                 | `read_cr0()` 实时读取         |
| `HOST_CR3`      | 当前 host CR3 (0xA000)        | host 页目录                   |
| `HOST_CR4`      | 当前 host CR4                 | 含 VMXE 位                   |
| `HOST_SEL_CS`   | `0x08` (KERNEL_CODE_SEG)      | 返回 host 内核代码段          |
| `HOST_SEL_SS/DS/ES/FS/GS` | `0x10` (KERNEL_DATA_SEG) |                        |
| `HOST_SEL_TR`   | `0x80` (KERNEL_TSS)           | host TSS                      |
| `HOST_BASE_TR`  | `0x9080`                      | GDT 基址 + TSS 槽偏移         |
| `HOST_BASE_GDTR`| `0x9000` (gdt_table)          | host GDT 基址                 |
| `HOST_BASE_IDTR`| `0xB000` (idt_table)          | host IDT 基址                 |
| `HOST_SYSENTER_EIP` | `&entry_sysenter`         | guest sysenter 的 host 处理入口|
| `HOST_SYSENTER_CS` | `0x08`                    |                               |
| **`HOST_RIP`**  | **`&vmx_return`**             | **VM exit 后跳转的 host 函数**|
| `HOST_RSP`      | 动态写入（vmx_enter_guest 中）| 写当前 esp，每次 vmlaunch 前更新|

`vmx_return` 标签位于 `vmx_enter_guest()` 内联汇编中，VM exit 后先执行 `SAVE_GPR_C` 保存 guest 通用寄存器，再返回 C 函数 `exit_handler()`。

---

## 四、VMCS Guest State（VM entry 时载入的 Guest 寄存器）

### 4.1 控制寄存器

| 字段          | 值                              | 说明                               |
|---------------|---------------------------------|------------------------------------|
| `GUEST_CR0`   | `host CR0 \| X86_CR0_WP`       | 继承 host，强制开启写保护 (WP=1)   |
| `GUEST_CR3`   | `guest_page_dir` 物理地址       | 独立 guest 页表，由 `setup_guest_page_tables()` 构建 |
| `GUEST_CR4`   | `host CR4 & ~PCIDE`             | 继承 host，清除 PCIDE              |

**CR0.WP = 1 的意义**：即使是特权级 0 的代码，也不能向只读页（PTE.W=0）写入，否则触发 #PF。这是页表测试（测试5）的核心机制。

### 4.2 段寄存器（与 host 共享 GDT）

```
CS:  选择子=0x08  AR=0xC09B  基址=0  限长=0xFFFFFFFF
SS:  选择子=0x10  AR=0xC093  基址=0  限长=0xFFFFFFFF
DS/ES/FS/GS: 同 SS
TR:  选择子=0x80  AR=0x8B   基址=0x9080  限长=0xFFFF
LDTR: 选择子=0  AR=0x82 (无效)
```

guest 与 host 使用**同一套 GDT 和 IDT**，地址相同（GDT=0x9000, IDT=0xB000）。这意味着 guest 内的段描述符与 host 完全相同。

### 4.3 执行上下文

| 字段             | 值                          | 说明                     |
|------------------|-----------------------------|--------------------------|
| `GUEST_RIP`      | `&guest_entry`              | guest 代码入口           |
| `GUEST_RSP`      | `guest_stack + 4096`        | guest 栈顶（4KB 栈）     |
| `GUEST_RFLAGS`   | `0x2`                       | 仅 fixed bit=1，IF=0     |
| `GUEST_SYSENTER_EIP` | `&entry_sysenter`      | guest sysenter 处理入口  |
| `GUEST_SYSENTER_ESP` | `guest_syscall_stack + 4096` | sysenter 专用栈     |
| `GUEST_SYSENTER_CS`  | `0x08`                 | KERNEL_CODE_SEG          |
| `GUEST_ACTV_STATE`   | `ACTV_ACTIVE (0)`      | guest 正常运行态         |
| `GUEST_INTR_STATE`   | `0`                    | 无被阻塞中断             |

---

## 五、Guest 页表（`setup_guest_page_tables()`）

### 5.1 结构

```
guest_page_dir[1024]  (4KB 对齐, 在内核 BSS)
guest_page_table[1024] (4KB 对齐, 在内核 BSS)

guest_page_dir[0]   = &guest_page_table | P|W|U
                      → 覆盖 [0x000000 ~ 0x3FFFFF]，使用 4KB 小页

guest_page_dir[1]   = 0x400000 | P|W|U|PS  (4MB 大页)
guest_page_dir[2]   = 0x800000 | P|W|U|PS
...
guest_page_dir[1023] = ...                  (4MB 大页恒等映射)
```

### 5.2 第 0 号区域的 4KB 细粒度映射

```
guest_page_table[i] = (i << 12) | P|W|U   (i = 0..1023，恒等映射)

例外：
guest_page_table[0x103] = 0x103000 | P|U   ← 清除 W 位！只读！
```

**只读页计算**：
```
TEST_READONLY_ADDR = 0x103B46
页号 = 0x103B46 >> 12 = 0x103 (= 259)
页基址 = 0x103000
```

因此地址 `0x103000 ~ 0x103FFF` 这整个 4KB 页被设为只读。`guest_entry` 本身就位于内核代码段（只读的代码页）附近，测试通过写这个地址触发 #PF。

### 5.3 Guest 页表与 Host 页表的关系

```
         Host (CR3=0xA000)          Guest (CR3=guest_page_dir)
         ─────────────────          ──────────────────────────
第0个4MB  page_dir[0]=4MB大页        page_dir[0]→page_table (4KB小页)
           恒等映射，P|W              第0x103页只读，其余P|W
其余       未映射                     4MB大页恒等映射
```

两者共享同一物理内存，但 guest 对 0x103000 页的写权限被单独收窄。

---

## 六、Guest 代码 `guest_entry` 的测试片段

`guest_entry` 位于 `src/kernel/vm/vm_as.S`，按顺序执行 4 个测试，每个测试都会触发一次或多次 VM exit，由 `exit_handler()` 处理后 VMRESUME 继续。

---

### 测试 4：CR3 读写拦截

```asm
mov $0x111000, %eax    # 只是往 eax 装一个值，不触发 exit
mov %cr3, %ebx         # ← MOV reg, CR3  → VM exit (reason 29, CR_STORE)
mov %eax, %cr3         # ← MOV CR3, reg  → VM exit (reason 28, CR_LOAD)
```

**触发原因**：VMCS `CPU_EXEC_CTRL0` 开启了 `CPU_CR3_STORE`(bit16) 和 `CPU_CR3_LOAD`(bit15)。

**exit_handler 处理（case 28/29）**：
1. 读 `EXI_QUALIFICATION` 获取访问类型（bit4: 0=读CR, 1=写CR）
2. 读 `EXI_INST_LEN` 获取指令字节数
3. `GUEST_RIP += inst_len`（跳过该指令）
4. 不真正模拟 CR3 操作，guest CR3 保持不变
5. 返回 `VMX_RESUME`

**目的**：验证 VMM 可以拦截 guest 对 CR3 的访问，是虚拟化 guest 地址空间切换的基础能力。

---

### 测试 5：页表只读保护（#PF）

```asm
mov test_no_access_page_addr, %eax  # eax = TEST_READONLY_ADDR = 0x103B46
movl $0x42, (%eax)                  # 向只读页写入 → #PF
```

**触发链**：
```
guest 执行 MOV [0x103B46], 0x42
  ↓
guest CR0.WP = 1，PTE[0x103].W = 0
  ↓
CPU 产生 #PF (向量 14)
  ↓
EXC_BITMAP bit14 = 1 → VM exit (reason 0, Exception/NMI)
```

**exit_handler 处理（case 0）**：
1. 读 `EXI_INTR_INFO` 取异常向量号（byte 0），确认是向量 14
2. 读 `read_cr2()` 获取触发异常的虚拟地址（= 0x103B46）
3. 打印 `"PAGE FAULT: Attempted write to read-only page at 0x103B46"`
4. 将 `GUEST_CR3` 切换为 host 的 CR3（`read_cr3()`）模拟"修复"
5. 返回 `VMX_RESUME`（guest 继续，但下一条指令仍是触发 #PF 的那条）

**注意**：当前处理只打印信息并替换 CR3，并未真正修复 PTE 的 W 位，因此若 VMRESUME 后 guest 重试同一条指令，会再次触发 #PF。但由于 guest 接下来执行 `hlt`，不会死循环。

---

### 测试 6：HLT 拦截

```asm
hlt    # → VM exit (reason 12, HLT)
```

**触发原因**：`CPU_EXEC_CTRL0` 开启了 `CPU_HLT`(bit7)。

**exit_handler 处理（case 12）**：
1. 打印 `"HLT VMEXIT: RIP=0x..."`
2. `GUEST_RIP += 1`（HLT 指令长度为 1 字节，跳过它）
3. 返回 `VMX_RESUME`

**目的**：HLT 是 guest 等待中断的方式。拦截它允许 VMM 控制 guest 的空闲行为（如调度其他虚拟 CPU），而不让物理 CPU 真正停止。

---

### 测试 7：VMCALL Hypercall（正常退出）

```asm
mov $1, %edi       # 参数：HYPERCALL_VMEXIT = 1
call hypercall     # → 设置 hypercall_field，执行 vmcall
```

`hypercall()` 函数（vm.c）执行过程：
```c
val = (1 & HYPERCALL_MASK) | HYPERCALL_BIT;  // = 0x1001
hypercall_field = val;
asm("vmcall");   // → VM exit (reason 18, VMCALL)
```

**exit_handler 处理**：
1. `is_hypercall()` 检查 reason == 18，返回 true
2. `GUEST_RIP += 3`（VMCALL 指令长度为 3 字节）
3. `handle_hypercall()` 读取 `hypercall_field & HYPERCALL_MASK = 1`
4. 匹配 `HYPERCALL_VMEXIT`：打印 `"hypercall: VMEXIT"`，返回 `VMX_VMEXIT`

**vmx_run 循环**收到 `VMX_VMEXIT`：
```c
case VMX_VMEXIT:
    guest_finished = 1;
    printf("Guest exited normally");
    return 0;   // 退出 vmx_run 循环
```

之后执行 `vmx_off()`，离开 VMX root 模式，`virt_enable()` 返回，`init_as.S` 执行 `jmp .`。

---

## 七、VM exit / VMRESUME 状态机

```
vmx_run():
  ┌──────────────────────────────────────────────────────┐
  │  vmx_enter_guest()                                   │
  │    vmwrite HOST_RSP = esp                            │
  │    LOAD_GPR_C  (恢复 guest 通用寄存器)               │
  │    launched==0 → VMLAUNCH                            │
  │    launched==1 → VMRESUME                            │
  │             ↓ (guest 运行中...)                       │
  │         VM exit 发生                                 │
  │             ↓                                        │
  │    vmx_return: SAVE_GPR_C (保存 guest 通用寄存器)    │
  │  ← 返回 entered=true                                 │
  │                                                      │
  │  launched = 1                                        │
  │  ret = exit_handler()                                │
  │    → VMX_RESUME: continue (回到循环顶部 VMRESUME)   │
  │    → VMX_VMEXIT: guest_finished=1, return 0         │
  │    → VMX_EXIT:   print error, break                 │
  └──────────────────────────────────────────────────────┘
```

---

## 八、Guest 通用寄存器保存/恢复机制

**保存宏** `SAVE_GPR_C`（vm.c 内联汇编）：
```asm
xchg %eax, regs+0x00   # struct regs.eax
xchg %ebx, regs+0x04   # struct regs.ebx
xchg %ecx, regs+0x08   # struct regs.ecx
xchg %edx, regs+0x0c   # struct regs.edx
xchg %ebp, regs+0x10   # struct regs.ebp
xchg %esi, regs+0x14   # struct regs.esi
xchg %edi, regs+0x18   # struct regs.edi
```

- `LOAD_GPR_C` 与 `SAVE_GPR_C` 相同（xchg 对称，load=save）
- ESP 通过 `HOST_RSP` VMCS 字段保存/恢复
- EFLAGS 通过 `GUEST_RFLAGS` VMCS 字段读写
- CR2 通过 `read_cr2()` 在异常处理时单独读取（不在 `regs` 结构体中）
