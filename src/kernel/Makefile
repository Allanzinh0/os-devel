BUILD_DIR?=build/
ASM?=nasm

.PHONY: all clean

all: kernel

kernel: $(OBJ_DIR)/kernel.bin
	
$(OBJ_DIR)/kernel.bin:
	$(ASM) main.asm -f bin -o $(OBJ_DIR)/kernel.bin

clean:
	rm -f $(OBJ_DIR)/kernel.bin
