CC = aarch64-linux-gnu-gcc
LINKER = aarch64-linux-gnu-ld
OBJ_CPY = aarch64-linux-gnu-objcopy

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -g
INC = -Iinclude/kernel/ -Iinclude/lib/

OBJ_DIR = objs
SRC_DIR = src


SRCS = $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/*/*.S)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o,$(SRCS))
OBJS := $(patsubst $(SRC_DIR)/%.S, $(OBJ_DIR)/%.o, $(OBJS))

KERFIL = $(OBJ_DIR)/bootloader/start.o $(OBJ_DIR)/bootloader/bootloader.o

BOOTFIL = $(filter $(OBJ_DIR)/bootloader/%.o,$(OBJS))
BOOTFIL += $(OBJ_DIR)/lib/string.o $(OBJ_DIR)/lib/uart.o

all: kernel8.img bootloader.img

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

kernel8.img: $(OBJS) 
	$(LINKER) -nostdlib -nostartfiles $(filter-out $(KERFIL),$(OBJS)) -T $(SRC_DIR)/kernel/link.ld -o kernel8.elf
	$(OBJ_CPY) -O binary kernel8.elf kernel8.img

bootloader.img: $(OBJS)
	$(LINKER) -nostdlib -nostartfiles $(BOOTFIL) -T $(SRC_DIR)/bootloader/link.ld -o bootloader.elf
	$(OBJ_CPY) -O binary bootloader.elf bootloader.img

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio

initrd:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -serial null -serial pty -initrd initramfs.cpio

stdinitrd:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio

fat:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -initrd initramfs.cpio -drive if=sd,file=sfn_nctuos.img,format=raw

debug:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -S -s -serial null -serial stdio -initrd initramfs.cpio -drive if=sd,file=sfn_nctuos.img,format=raw

test:
	@echo $(BOOTFIL)

clean:
	rm -rf build objs *r.img *8.img *.elf 2>/dev/null

