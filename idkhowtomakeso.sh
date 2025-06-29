mkdir tmp

echo "compiling..."
nasm -f elf32 kernel.asm -o tmp/kasm.o

CFLAGS="-m32 -c -fno-stack-protector"
gcc $CFLAGS kernel.c -o tmp/kernel.o
gcc $CFLAGS drivers/keyboard/keyboard_driver.c -o tmp/keyboard_driver.o
nasm -f elf32 drivers/disk_driver/disk_driver.S -o tmp/disk_driver.o
gcc $CFLAGS mylibs/my_stdlib.c -o tmp/my_stdlib.o
gcc $CFLAGS shell/shell.c -o tmp/shell.o
gcc $CFLAGS shell/commands/commands.c -o tmp/commands.o
gcc $CFLAGS fs/file_system.c -o tmp/file_system.o
gcc $CFLAGS fs/diskio.c -o tmp/diskio.o

echo "linking..."
ld -m elf_i386 -T link.ld -o tmp/kernel \
tmp/kasm.o \
tmp/kernel.o \
tmp/keyboard_driver.o \
tmp/disk_driver.o \
tmp/shell.o \
tmp/my_stdlib.o \
tmp/commands.o \
tmp/file_system.o \
tmp/diskio.o

echo "creating disk"
dd if=/dev/zero of=disk.img bs=1M count=100

echo "starting emulator"
qemu-system-i386 \
-kernel tmp/kernel \
-drive file=disk.img,format=raw,if=ide,index=0,media=disk

echo "rming tmp"
rm -rf tmp
