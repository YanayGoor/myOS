# $@ = target file
# $< = first dependency
# $^ = all dependencies

C_SOURCES = $(wildcard kernel/*.c kernel/*/*.c kernel/*/*/*.c drivers/*.c drivers/*/*.c cpu/*.c)
HEADERS = $(wildcard kernel/*.h kernel/*/*.h kernel/*/*/*.h drivers/*.h drivers/*/*.h cpu/*.h)

OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o}

GCC = /usr/local/i386elfgcc/bin/i386-elf-gcc
LD = /usr/local/i386elfgcc/bin/i386-elf-ld
GDB = gdb

CFLAGS = -g

# First rule is the one executed when no parameters are fed to the Makefile
all: run

%.o: %.c ${HEADERS}
	${GCC} ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

kernel/kernel.bin: boot_sector/32bit/kernel_entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x1000 $^ --oformat binary

# debugging
kernel/kernel.elf: boot_sector/32bit/kernel_entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x1000 $^

boot_sector/boot_sector.bin: ./boot_sector/boot_sector.asm
	nasm -f bin $^ -o $@

os-image.bin: boot_sector/boot_sector.bin kernel/kernel.bin
	rm $@ &
	cp empty_hhd.bin $@ &
	cat $^ > $@

run: os-image.bin fs
	qemu-system-i386 -drive id=disk,file=$<,if=none,format=raw  -device ahci,id=ahci  -device ide-drive,drive=disk,bus=ahci.0


debug: os-image.bin fs kernel/kernel.elf
	qemu-system-i386 -s -drive id=disk,file=$<,if=none,format=raw  -device ahci,id=ahci  -device ide-drive,drive=disk,bus=ahci.0 -d guest_errors,int &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel/kernel.elf"

fs-image.bin:
	dd if=/dev/zero of="fs-image.bin" bs=1024k count=650
	/sbin/mkfs.ext2 -b 1024 fs-image.bin

fs: fs-image.bin
	dd if="fs-image.bin" of="os-image.bin" bs=512 seek=255


clean:
	rm -rf *.o *.dis os-image.bin *.elf
	rm -rf kernel/*.o boot_sector/*.bin drivers/*.o drivers/*/*.o boot/*.o cpu/*.o
