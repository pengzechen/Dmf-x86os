

TOOL_PREFIX = x86_64-elf-

BUILD_DIR = build/basic_set
KERNEL_BUILD_DIR = build/kernel

INCLUDE = -I ./include

CFLAGS = -g -c -O0 -m32 -fno-pie -fno-stack-protector -nostdlib -nostdinc

basic: src/basic_set/os.c src/basic_set/start.S
	rm -f build/disk.img
	fsutil file createnew build/disk.img 52428800

	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/basic_set/start.S  -o $(BUILD_DIR)/start.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/basic_set/os.c     -o $(BUILD_DIR)/os.o

	$(TOOL_PREFIX)ld -m elf_i386 -T src/basic_set/os.lds   $(BUILD_DIR)/start.o  $(BUILD_DIR)/os.o   -o $(BUILD_DIR)/os.elf
	
	$(TOOL_PREFIX)objdump -x -d -S $(BUILD_DIR)/os.elf > $(BUILD_DIR)/os_dis.txt
	$(TOOL_PREFIX)objcopy -O binary $(BUILD_DIR)/os.elf $(BUILD_DIR)/os.bin
	$(TOOL_PREFIX)readelf -a $(BUILD_DIR)/os.elf > $(BUILD_DIR)/os_elf.txt

	dd if=$(BUILD_DIR)/os.bin   of=build/disk.img bs=512  conv=notrunc



kernel: src/kernel/init_as.S src/kernel/irq_as.S src/kernel/base.c \
	   src/kernel/irq.c src/kernel/mem.c src/kernel/task.c src/kernel/init.c \
	   src/kernel/io.c src/kernel/string.c

	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/init_as.S     -o $(KERNEL_BUILD_DIR)/init_as.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/irq_as.S     -o $(KERNEL_BUILD_DIR)/irq_as.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/base.c     -o $(KERNEL_BUILD_DIR)/base.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/irq.c     -o $(KERNEL_BUILD_DIR)/irq.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/mem.c     -o $(KERNEL_BUILD_DIR)/mem.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/task.c     -o $(KERNEL_BUILD_DIR)/task.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/init.c     -o $(KERNEL_BUILD_DIR)/init.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/io.c        -o $(KERNEL_BUILD_DIR)/io.o
	$(TOOL_PREFIX)gcc $(INCLUDE) $(CFLAGS) src/kernel/string.c     -o $(KERNEL_BUILD_DIR)/string.o



	$(TOOL_PREFIX)ld -m elf_i386 -T src/kernel/kernel.lds   $(KERNEL_BUILD_DIR)/init_as.o   	\
		$(KERNEL_BUILD_DIR)/irq_as.o $(KERNEL_BUILD_DIR)/base.o $(KERNEL_BUILD_DIR)/irq.o   	\
		$(KERNEL_BUILD_DIR)/mem.o $(KERNEL_BUILD_DIR)/task.o $(KERNEL_BUILD_DIR)/init.o			\
		$(KERNEL_BUILD_DIR)/io.o $(KERNEL_BUILD_DIR)/string.o                                   \
		-o $(KERNEL_BUILD_DIR)/kernel.elf
	
	$(TOOL_PREFIX)objdump -x -d -S $(KERNEL_BUILD_DIR)/kernel.elf > $(KERNEL_BUILD_DIR)/kernel_dis.txt
	$(TOOL_PREFIX)objcopy -O binary $(KERNEL_BUILD_DIR)/kernel.elf $(KERNEL_BUILD_DIR)/kernel.bin
	$(TOOL_PREFIX)readelf -a $(KERNEL_BUILD_DIR)/kernel.elf > $(KERNEL_BUILD_DIR)/kernel_elf.txt

	dd if=$(KERNEL_BUILD_DIR)/kernel.bin   of=build/disk.img bs=512  conv=notrunc  seek=100



clean:
	rm -f $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.txt $(BUILD_DIR)/*.bin
	rm -f $(KERNEL_BUILD_DIR)/*.elf $(KERNEL_BUILD_DIR)/*.o $(KERNEL_BUILD_DIR)/*.txt $(KERNEL_BUILD_DIR)/*.bin
	

	# fsutil file createnew disk.img 52428800
