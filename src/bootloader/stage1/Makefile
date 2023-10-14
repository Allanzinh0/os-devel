BUILD_DIR?=build
OBJ_DIR?=obj
ASM?=nasm

.PHONY: all clean

all: stage1

stage1: $(OBJ_DIR)/stage1.bin
	
$(OBJ_DIR)/stage1.bin: always
	$(ASM) boot.asm -f bin -o $(OBJ_DIR)/stage1.bin

always:
	mkdir -p $(OBJ_DIR)

clean:
	rm -f $(OBJ_DIR)/stage1.bin
