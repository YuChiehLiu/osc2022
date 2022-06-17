CC = aarch64-linux-gnu-gcc
LINKER = aarch64-linux-gnu-ld
OBJ_CPY = aarch64-linux-gnu-objcopy

all: usrpgm.img move

usrpgm.o: usrpgm.S
	$(CC) -c $< -o $@

usrpgm.img: usrpgm.o
	$(LINKER) usrpgm.o -T linker.ld -o usrpgm.elf
	$(OBJ_CPY) -O binary usrpgm.elf usrpgm.img

move:
	mv usrpgm.img ../rootfs

clean:
	rm -rf *.elf *img *.o ../rootfs/usrpgm.img ../initramfs.cpio 2>/dev/null