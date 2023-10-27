export MAKE_DISK_SIZE=$$(( 64 * 1024 * 1024 ))
export MAKE_PART_FORMAT=32

#
# Floppy Image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: $(BUILD_DIR)/bootloader/stage1.bin $(BUILD_DIR)/bootloader/stage2.bin $(BUILD_DIR)/kernel/kernel.elf
	@./build-tools/make_floppy.sh $@

docker_floppy:
	@docker build --file Dockerfile --build-arg MAKE_CMD=floppy_image --output $(BUILD_DIR) .

run_floppy: docker_floppy
	@qemu-system-i386 -m 32 -debugcon stdio -fda ${BUILD_DIR}/main_floppy.img

#
# Disk Image
#
disk_image: $(BUILD_DIR)/main_disk.raw

$(BUILD_DIR)/main_disk.raw: $(BUILD_DIR)/bootloader/stage1.bin $(BUILD_DIR)/bootloader/stage2.bin $(BUILD_DIR)/kernel/kernel.elf
	@./build-tools/make_disk.sh $@ $(MAKE_DISK_SIZE)

docker_disk:
	@docker build --file Dockerfile --build-arg MAKE_CMD=disk_image --output $(BUILD_DIR) .

run_disk: docker_disk
	@qemu-system-i386 -m 32 -debugcon stdio -drive file=$(BUILD_DIR)/main_disk.raw,format=raw,index=0,media=disk

debug_disk: docker_disk
	gdb -x debug_disk.gdb
