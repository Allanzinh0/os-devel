b *0x7C00
set disassembly-flavor intel
symbol-file build/kernel/kernel.elf
symbol-file build/bootloader/stage2.elf
layout split
layout reg
target remote | qemu-system-i386 -S -gdb stdio -m 32 -hda build/main_disk.raw
