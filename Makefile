include build-tools/config.mk

.PHONY: all floppy_image kernel bootloader tools_fat clean always run docker

all: always floppy_image bootloader kernel
#
# Floppy Image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/main_floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "OS-DEVEL" $(BUILD_DIR)/main_floppy.img
	dd if=$(OBJ_DIR)/stage1.bin of=$(BUILD_DIR)/main_floppy.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main_floppy.img $(OBJ_DIR)/stage2.bin "::stage2.bin"
	mcopy -i $(BUILD_DIR)/main_floppy.img $(OBJ_DIR)/kernel.bin "::kernel.bin"
	mcopy -i $(BUILD_DIR)/main_floppy.img test.txt "::test.txt"

#
# Bootloader
#
bootloader: stage1 stage2

stage1: $(OBJ_DIR)/stage1.bin

$(OBJ_DIR)/stage1.bin: always
	$(MAKE) -C src/bootloader/stage1

stage2: $(OBJ_DIR)/stage2.bin

$(OBJ_DIR)/stage2.bin: always
	$(MAKE) -C src/bootloader/stage2

#
# Kernel
#
kernel: $(OBJ_DIR)/kernel.bin

$(OBJ_DIR)/kernel.bin: always
	$(MAKE) -C src/kernel

docker:
	docker build --file Dockerfile --output $(BUILD_DIR) .

#
# Tools
#
tools_fat: $(BUILD_DIR)/tools/fat

$(BUILD_DIR)/tools/fat: $(TOOLS_DIR)/fat/fat.c
	mkdir -p $(BUILD_DIR)/tools
	$(CC) -g -o $(BUILD_DIR)/tools/fat $(TOOLS_DIR)/fat/fat.c

#
# Always
#
always:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)

#
# Clean
#
clean:
	$(MAKE) -C src/bootloader/stage1 clean
	$(MAKE) -C src/bootloader/stage2 clean
	$(MAKE) -C src/kernel clean
	rm -rf $(BUILD_DIR)
	rm -rf $(OBJ_DIR)

#
# Run
#
run:
	qemu-system-i386 -fda $(BUILD_DIR)/main_floppy.img
