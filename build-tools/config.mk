export TARGET = i686-elf
export TARGET_ASM = nasm
export TARGET_ASMFLAGS = 
export TARGET_CFLAGS = -std=c99 -O2 -ffreestanding -nostdlib -g
export TARGET_CXXFLAGS = -std=c++17 -O2 -ffreestanding -nostdlib -g
export TARGET_CC = $(TARGET)-gcc
export TARGET_CXX = $(TARGET)-g++
export TARGET_LD = $(TARGET)-g++
export TARGET_AR = $(TARGET)-ar
export TARGET_LINKFLAGS = 
export TARGET_STRIP = $(TARGET)-strip
export TARGET_LIBS = 

export BUILD_DIR = $(abspath build)
export OBJ_DIR = $(abspath obj)
export SOURCE_DIR = ./src/
