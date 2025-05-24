mkdir tmp

echo "nasm -f elf32 kernel.asm -o tmp/kasm.o"
nasm -f elf32 kernel.asm -o tmp/kasm.o

echo "compiling..."
gcc -m32 -c kernel.c -o tmp/kernel.o
gcc -m32 -c drivers/keyboard/keyboard_driver.c -o tmp/keyboard_driver.o
gcc -m32 -c drivers/disk_driver/disk_driver.c -o tmp/disk_driver.o
gcc -m32 -c mylibs/my_stdlib.c -o tmp/my_stdlib.o
gcc -m32 -c shell/shell.c -o tmp/shell.o
gcc -m32 -c shell/commands/commands.c -o tmp/commands.o

echo "linking..."
ld -m elf_i386 -T link.ld -o tmp/kernel \
tmp/kasm.o \
tmp/kernel.o \
tmp/keyboard_driver.o \
tmp/shell.o \
tmp/my_stdlib.o \
tmp/commands.o \
tmp/disk_driver.o

echo "dd if=/dev/zero of=disk.img bs=1M count=100"
dd if=/dev/zero of=disk.img bs=1M count=100

echo "qemu-system-i386 \
-kernel tmp/kernel \
-drive file=disk.img,format=raw,if=ide,index=0,media=disk "
qemu-system-i386 \
-kernel tmp/kernel \
-drive file=disk.img,format=raw,if=ide,index=0,media=disk

echo "rming tmp"
rm -rf tmp
