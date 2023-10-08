BUILD_DIR?=build/
ASM?=nasm

.PHONY: all clean

all: stage1

stage1: $(BUILD_DIR)/stage1.bin
	
$(BUILD_DIR)/stage1.bin: always
	$(ASM) boot.asm -f bin -o $(BUILD_DIR)/stage1.bin

always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -f $(BUILD_DIR)/stage1.bin
