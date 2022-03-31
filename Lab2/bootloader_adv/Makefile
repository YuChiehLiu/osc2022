#
# Copyright (C) 2018 bzt (bztsrc@github)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
#

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
CC = aarch64-linux-gnu-gcc
LINKER = aarch64-linux-gnu-ld
OBJ_CPY = aarch64-linux-gnu-objcopy
EXE = bootloader


all: clean $(EXE).img

start.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE).img: start.o $(OBJS)
	$(LINKER) -nostdlib -nostartfiles start.o $(OBJS) -T link.ld -o $(EXE).elf
	$(OBJ_CPY) -O binary $(EXE).elf $(EXE).img

clean:
	rm $(EXE).elf $(EXE).img *.o >/dev/null 2>/dev/null || true

pty:
	qemu-system-aarch64 -M raspi3 -kernel $(EXE).img -serial null -serial pty

run:
	qemu-system-aarch64 -M raspi3 -kernel $(EXE).img -serial null -serial stdio

initrd:
	qemu-system-aarch64 -M raspi3 -kernel $(EXE).img -serial null -serial pty -initrd initramfs.cpio
