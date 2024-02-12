
#include "os.h"
#include "types.h"
#include "cpu.h"
#include "bios.h"
#include "elf.h"

// 2048  size: 0x800  2k
gdt_table_t
gdt_table[256] = {

    {0x0000, 0x0000, 0x0000, 0x0000},    
    {0xffff, 0x0000, 0x9a00, 0x00cf},   
    {0xffff, 0x0000, 0x9200, 0x00cf},   

    {0xffff, 0x0000, 0xfa00, 0x00cf},    // task 0 code 24
    {0xffff, 0x0000, 0xf300, 0x00cf},    // task 0 data 32

    // {0x86,   0x0000, 0xe900, 0x0000},    // task 0 tss  TASK_0_TSS
    // {0x86,   0x0000, 0xe900, 0x0000},

};  

// 2048 size: 0x800  2k
idt_table_t
idt_table[256] = {0};

// 4k
page_dir_t
page_dir[1024] OS_ALIGN(4096) = {
    PDE_P|PDE_W|PDE_U|PDE_PS|0
};

// 4k
page_table_t
page_table[1024] OS_ALIGN(4096) = {0};

static uint32_t reload_elf_file (uint8_t * file_buffer) {
    Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *)file_buffer;
    
    if ((elf_hdr->e_ident[0] != ELF_MAGIC) || (elf_hdr->e_ident[1] != 'E')
        || (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')) {
        return 0;
    }

    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }

		dest= (uint8_t *)phdr->p_paddr + phdr->p_filesz;
		for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
			*dest++ = 0;
		}
    }

    return elf_hdr->e_entry;
}

static void read_disk(int sector, int sector_count, uint8_t * buf) {
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_count >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_count));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

	uint16_t *data_buf = (uint16_t*) buf;
	while (sector_count-- > 0) {
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		for (int i = 0; i < 512 / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}

void os_init() {
    read_disk(100, 500, (uint8_t *)0x100000);
}
