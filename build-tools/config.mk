export MAKE_DISK_SIZE=$$(( 64 * 1024 * 1024 ))
export MAKE_PART_FORMAT=32

export TARGET = i686-elf
export TARGET_ASM = nasm
export TARGET_ASMFLAGS = 
export TARGET_CFLAGS = -std=c99 -g -O2
export TARGET_CC = $(TARGET)-gcc
export TARGET_CXX = $(TARGET)-g++
export TARGET_LD = $(TARGET)-gcc
export TARGET_LINKFLAGS = 
export TARGET_LIBS = 

export BUILD_DIR = $(abspath build)
export OBJ_DIR = $(abspath obj)
