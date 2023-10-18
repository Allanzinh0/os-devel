# OS Development
---
This repository is only for study purposes!

The code is following the playlist below from nanobyte:
[Link for the playlist](https://www.youtube.com/playlist?list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN)


The OS is able to run from a floppy disk, print messages on screen read the kernel from disk and run!
The Bootloader is able to access the floppy drive find the kernel and run!
The Kernel has its own GDT and have an implementation of IDT, ISR and hardware (IRQ) interrupts and exceptions!
The kernel also have an printf function to print on screen.

For the compilation is used an docker file that create a toolchain using alpine linux!

## Languages used
 * Assembly
 * C
 * Docker

## Compilers
 * NASM
 * GCC

## Disk format
 * FAT12
