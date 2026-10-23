#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

# Required on Ubuntu/Debian:
# sudo apt update
# sudo apt install build-essential gcc-multilib grub-pc-bin xorriso mtools

mkdir -p build iso/boot/grub
gcc -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
    -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables \
    -c src/kernel.c -o build/kernel.o

gcc -m32 -ffreestanding -fno-pie -c src/boot.s -o build/boot.o

ld -m elf_i386 -T src/linker.ld -nostdlib \
    build/boot.o build/kernel.o -o iso/boot/kernel.elf

grub-file --is-x86-multiboot iso/boot/kernel.elf

grub-mkrescue -o NanoSoft-DOS.iso iso

echo
echo "Success! ISO created at: $(pwd)/NanoSoft-DOS.iso"
