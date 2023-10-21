include build-tools/config.mk

.PHONY: all floppy_image kernel bootloader tools_fat clean always run docker

all: clean always floppy_image bootloader kernel

#
# Floppy Image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	./build-tools/make_floppy.sh $@

#
# Disk Image
#
disk_image: $(BUILD_DIR)/main_disk.raw

$(BUILD_DIR)/main_disk.raw: bootloader kernel
	./build-tools/make_disk.sh $@ $(MAKE_DISK_SIZE)

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
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
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
run_floppy:
	qemu-system-i386 -m 32 -debugcon stdio -fda $(BUILD_DIR)/main_floppy.img

run_disk:
	qemu-system-i386 -m 32 -debugcon stdio -drive file=$(BUILD_DIR)/main_disk.raw,format=raw,index=0,media=disk

#
# Debug
#
debug_disk:
	gdb -x debug_disk.gdb
