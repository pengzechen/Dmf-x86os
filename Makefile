

TOOL_PREFIX = x86_64-elf-

BUILD_DIR = build

CFLAGS = -g -c -O0 -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc

all: src/os.c src/os.h src/cfg.h src/types.h src/cpu.h src/start.S

	$(TOOL_PREFIX)gcc $(CFLAGS) src/start.S  -o $(BUILD_DIR)/start.o
	$(TOOL_PREFIX)gcc $(CFLAGS) src/os.c     -o $(BUILD_DIR)/os.o

	$(TOOL_PREFIX)ld -m elf_i386 -T src/os.lds   $(BUILD_DIR)/start.o  $(BUILD_DIR)/os.o   -o $(BUILD_DIR)/os.elf

	$(TOOL_PREFIX)objdump -x -d -S $(BUILD_DIR)/os.elf > $(BUILD_DIR)/os_dis.txt
	$(TOOL_PREFIX)objcopy -O binary $(BUILD_DIR)/os.elf $(BUILD_DIR)/os.bin
	$(TOOL_PREFIX)readelf -a $(BUILD_DIR)/os.elf > $(BUILD_DIR)/os_elf.txt

	dd if=$(BUILD_DIR)/os.bin of=$(BUILD_DIR)/disk.img conv=notrunc

clean:
	rm -f $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.txt $(BUILD_DIR)/*.bin