CFLAGS="-m32 -c -fno-stack-protector -ffreestanding -fno-pie -no-pie"

mkdir tmp

echo "making bootloader..."
nasm -f bin   boot/bootloader.S                   -o tmp/bootloader.bin
nasm -f elf32 boot/boot_sec_stage.S               -o tmp/boot_sec_stage.o
nasm -f elf32 asm/io_asm.S                        -o tmp/io_asm.o

gcc $CFLAGS boot/boot_sec_stage.c -o tmp/boot_sec_stagec.o

ld -m elf_i386 -T boot/bLink.ld --oformat binary -o tmp/boot_sec_stage.bin \
tmp/boot_sec_stage.o \
tmp/boot_sec_stagec.o \
tmp/io_asm.o


echo "making kernel..."
nasm -f elf32 kernel/kernel.S                   -o tmp/kasm.o
#nasm -f elf32 asm/io_asm.S                      -o tmp/io_asm.o we have this from bootloader
nasm -f elf32 asm/write_read_port_asm.S         -o tmp/write_read_port_asm.o
nasm -f elf32 asm/mmu.S                         -o tmp/mmu.o

gcc $CFLAGS kernel/kernel.c                     -o tmp/kernel.o
gcc $CFLAGS drivers/keyboard/keyboard_driver.c  -o tmp/keyboard_driver.o
gcc $CFLAGS drivers/disk_driver/disk_driver.c   -o tmp/disk_driver.o
gcc $CFLAGS mylibs/my_stdlib.c                  -o tmp/my_stdlib.o
gcc $CFLAGS shell/shell.c                       -o tmp/shell.o
gcc $CFLAGS shell/commands/commands.c           -o tmp/commands.o
gcc $CFLAGS fs/file_system.c                    -o tmp/file_system.o
gcc $CFLAGS fs/fs_api.c                         -o tmp/fs_api.o
gcc $CFLAGS memory_management/heap.c            -o tmp/heap.o
gcc $CFLAGS memory_management/paging.c          -o tmp/paging.o

ld -m elf_i386 -T kernel/kLink.ld --oformat binary -o tmp/kernel.bin \
tmp/kasm.o \
tmp/io_asm.o \
tmp/write_read_port_asm.o \
tmp/mmu.o \
tmp/kernel.o \
tmp/keyboard_driver.o \
tmp/disk_driver.o \
tmp/shell.o \
tmp/my_stdlib.o \
tmp/commands.o \
tmp/file_system.o \
tmp/fs_api.o \
tmp/heap.o \
tmp/paging.o


BOOTLOADER_SEC_STAGE_SIZE=$(stat -c%s tmp/boot_sec_stage.bin)
KERNEL_START_OFFSET=$((1 + 1 + (( (BOOTLOADER_SEC_STAGE_SIZE + 511) / 512 ))))

dd if=/dev/zero of=disk.img bs=512 count=$((20*1024)) status=none

dd if=tmp/bootloader.bin        of=disk.img bs=512 count=1 conv=notrunc seek=0                    status=none
dd if=tmp/boot_sec_stage.bin    of=disk.img bs=512         conv=notrunc seek=1                    status=none
dd if=tmp/kernel.bin            of=disk.img bs=512         conv=notrunc seek=$KERNEL_START_OFFSET status=none

echo "starting emulator"
qemu-system-i386 \
-drive file=disk.img,format=raw,if=ide,index=0,media=disk

echo "rming tmp"
rm -rf tmp
