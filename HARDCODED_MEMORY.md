# 硬编码地址与内存布局文档

本文件汇总项目中所有硬编码的内存地址、端口、磁盘扇区偏移，以及与虚拟内存相关的关键常量。

---

## 一、完整物理内存布局

```
物理地址范围                大小     内容                        来源
──────────────────────────────────────────────────────────────────────────────
0x00000000 ~ 0x000003FF    1 KB     BIOS 中断向量表 (IVT)       BIOS
0x00000400 ~ 0x000004FF    256 B    BIOS Data Area              BIOS
0x00001000 ~ 0x000013FF    ~1 KB    E820 内存探测结果            detect_mem()
0x00007C00 ~ 0x00007DFF    512 B    MBR / 启动扇区 (_start 16位) 磁盘扇区 0
0x00007C00                  -       启动期栈顶 (向低地址生长)    start.S
0x00007E00 ~ 0x0000FDFF    32 KB    basic_set 代码体             磁盘扇区 2~65
0x00009000 ~ 0x000097FF    2 KB     GDT (gdt_table, 256×8B)     base.c / os.c
0x0000A000 ~ 0x0000AFFF    4 KB     Host 页目录 (page_dir)       os.c
0x0000B000 ~ 0x0000B7FF    2 KB     IDT (idt_table, 256×8B)     base.c / os.c
0x0000C000 ~ 0x0000CFFF    4 KB     Host 页表 (page_table)       os.c (预留)
0x00100000 ~ 0x007FFFFF    ~256 KB  内核二进制 (kernel.bin)      磁盘扇区 100~599
0x00200000                  -       页分配器 freelist 起始        alloc_page.c
0x00201000 ~ 0x00201FFF    4 KB     VMXON region (4K 对齐)       vm.c
0x00202000 ~ 0x00202FFF    4 KB     VMCS (4K 对齐)               vm.c
0x00300000 ~ 0x07CE0000    ~124 MB  物理内存分配池               mem.c
0x000B8000 ~ 0x000BFFFF    32 KB    VGA 文本模式显存              syscall.c
──────────────────────────────────────────────────────────────────────────────
```

**注**：guest 专属数据（guest_page_dir、guest_stack 等）位于内核 BSS 段内，
地址由链接器决定，不是固定硬编码，在此不列出具体地址。

---

## 二、磁盘扇区布局

```
扇区编号        内容                        读取方式
──────────────────────────────────────────────────────────────
0               MBR / basic_set 入口        BIOS INT 0x13 自动
2 ~ 65          basic_set 其余代码(64扇区)   INT 0x13, ah=0x02
100 ~ 599       内核二进制(500扇区=256KB)    ATA PIO, os_init()
──────────────────────────────────────────────────────────────
```

相关代码：
- `start.S:23` — `mov $64, %al` (读 64 扇区)，`mov $0x2, %cx` (从扇区 2 开始)
- `os.c:91` — `read_disk(100, 500, KERNEL_START_ADDR)`

---

## 三、GDT 布局（基地址 0x9000）

```
GDT 槽位   选择子(hex)   宏名             描述
──────────────────────────────────────────────────────────────────
[0]        0x0000        —                空描述符
[1]        0x0008        KERNEL_CODE_SEG  内核代码段  DPL=0, 32位, 4GB
[2]        0x0010        KERNEL_DATA_SEG  内核数据段  DPL=0, 32位, 4GB
[3]        0x001B        APP_CODE_SEG     用户代码段  DPL=3, 32位, 4GB
[4]        0x0023        APP_DATA_SEG     用户数据段  DPL=3, 32位, 4GB
[5]        0x0028        TASK_0_TSS       任务0 TSS
[6]        0x0030        TASK_1_TSS       任务1 TSS
[7]        0x0038        —                syscall 调用门 DPL=3
[16]       0x0080        KERNEL_TSS       内核 TSS (init_as.S ltr)
──────────────────────────────────────────────────────────────────
```

**关键推导**：KERNEL_TSS 选择子 = 16×8 = `0x80`，故 `HOST_BASE_TR = 0x9000 + 0x80 = 0x9080`。
该值在 `vm.c:init_vmcs_host()` 和 `init_vmcs_guest()` 中硬编码。

---

## 四、Host 页表（basic_set 建立，CR3 = 0xA000）

```c
// start.S: CR4 |= PSE (4MB 大页), CR3 = page_dir(0xA000)
page_dir[0] = 0x000000 | PDE_P | PDE_W | PDE_U | PDE_PS
              → 恒等映射 0x000000 ~ 0x3FFFFF (第一个 4MB)
page_dir[1..1023] = 0  → 未映射
```

覆盖范围涵盖：
- 启动代码 (0x7C00)、GDT/IDT/页表 (0x9000~0xC000)
- 内核入口 (0x100000)、VMXON/VMCS (0x201000/0x202000)
- VGA 显存 (0xB8000)

**限制**：物理分配池 0x300000~0x7CE0000 和内核 BSS 中的 guest 数据均在此 4MB 之外，
但 32 位保护模式下分页只做权限控制，内核段限长 4GB，访问这些地址不需要页表映射（段基址为 0，段限长为 0xFFFFFFFF）。

---

## 五、VMX 相关硬编码地址

| 常量/变量          | 值          | 说明                              | 位置        |
|--------------------|-------------|-----------------------------------|-------------|
| `vmxon_region`     | `0x201000`  | VMXON region，4KB，4K 对齐        | `vm.c:126`  |
| `vmcs`             | `0x202000`  | VMCS 结构体，4KB，4K 对齐         | `vm.c:212`  |
| `HOST_SEL_TR`      | `0x0080`    | 写入 VMCS 的 host TSS 选择子      | `vm.c:345`  |
| `HOST_BASE_TR`     | `0x9080`    | host TSS 描述符基址 (0x9000+0x80) | `vm.c:346`  |
| `GUEST_SEL_TR`     | `0x0080`    | guest TSS 选择子（与host共享GDT） | `vm.c:425`  |
| `GUEST_BASE_TR`    | `0x9080`    | guest TSS 基址                    | `vm.c:426`  |
| `GUEST_LIMIT_GDTR` | `0x07FF`    | GDT 限长 (256×8-1 = 2047)        | `vm.c:444`  |
| `GUEST_LIMIT_IDTR` | `0x07FF`    | IDT 限长                          | `vm.c:445`  |
| `TEST_READONLY_ADDR` | `0x103B46` | guest 页表测试用只读地址          | `vm.c:25`   |

---

## 六、物理内存分配池

```c
// mem.c:34
phys_alloc_init(0x300000, 0x7CE0000);
```

| 字段   | 值          | 说明            |
|--------|-------------|-----------------|
| 起始   | `0x300000`  | 3 MB            |
| 结束   | `0x7CE0000` | ~124 MB         |
| 大小   | ~121 MB     | 可用物理内存池  |

页分配器（当前未启用）freelist 起点硬编码于 `alloc_page.c:12`：`(void*)0x200000`。

---

## 七、I/O 端口硬编码

### 串口 UART (io.c)
```
基地址: 0x3F8 (COM1)
  +0x00: 数据寄存器 / 波特率低字节
  +0x01: 中断使能 / 波特率高字节
  +0x03: 行控制寄存器 (LCR)
  +0x05: 行状态寄存器 (LSR)
波特率: 115200 (除数 = 1)
```

### 8259A PIC (cfg.h / irq.c)
```
PIC0: 命令口 0x20, 数据口 0x21
PIC1: 命令口 0xA0, 数据口 0xA1
定时器 IRQ0 → IDT[0x20] (IRQ_PIC_START = 0x20)
```

### ATA PIO (os.c)
```
0x1F0: 数据寄存器 (16位读)
0x1F2: 扇区计数
0x1F3~0x1F5: LBA 地址
0x1F6: 驱动器/磁头 (0xE0 = LBA 模式)
0x1F7: 命令/状态寄存器 (0x24 = READ SECTORS EXT)
```

---

## 八、VMCS Guest 段描述符硬编码

guest 段全部继承 host 的段配置（共享同一套 GDT）：

```
CS: 选择子=0x08  基址=0  限长=0xFFFFFFFF  AR=0xC09B (32位代码, DPL=0, 可读可执行)
SS: 选择子=0x10  基址=0  限长=0xFFFFFFFF  AR=0xC093 (32位数据, DPL=0, 可读写)
DS/ES/FS/GS: 同 SS
LDTR: 选择子=0  AR=0x82 (64位, 无效)
TR:   选择子=0x80  基址=0x9080  限长=0xFFFF  AR=0x8B (32位 TSS, 忙)
```

AR（Access Rights）格式参考 Intel SDM Vol.3 Table 24-2。

---

## 九、内核 TSS 硬编码

`task.c` 中 `kernel_tss[]` 的关键硬编码：
- `esp0 = 0x7C00`：内核特权级 0 栈顶（复用启动栈）
- `ss0 = KERNEL_DATA_SEG`：内核数据段

---

## 十、段选择子速查

```c
// cfg.h
KERNEL_CODE_SEG = 0x08   // GDT[1], RPL=0
KERNEL_DATA_SEG = 0x10   // GDT[2], RPL=0
APP_CODE_SEG    = 0x1B   // GDT[3], RPL=3
APP_DATA_SEG    = 0x23   // GDT[4], RPL=3
TASK_0_TSS      = 0x28   // GDT[5]
TASK_1_TSS      = 0x30   // GDT[6]
KERNEL_TSS      = 0x80   // GDT[16]
```
