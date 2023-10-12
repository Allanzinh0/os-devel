# OS Development
---
This repository is only for study purposes!

The code is following the playlist below from nanobyte:
[Link for the playlist](https://www.youtube.com/playlist?list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN)

The current OS is able to print error messages on screen, read the second part of bootloader from disk and run it!
The second part of bootloader can also print on the screen using an basic implementation of printf function in 10h interrupt of BIOS.
The stage 2 of bootloader also have a simple FAT driver to access the entire floppy disk.

## Languages used
 * Assembly
 * C 16-bits real

## Compilers
 * NASM
 * Watcom C

## Disk format
 * FAT12
