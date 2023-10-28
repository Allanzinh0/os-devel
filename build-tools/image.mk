export MAKE_DISK_SIZE=$$(( 64 * 1024 * 1024 ))
export MAKE_PART_FORMAT=32

STAGE2_SECTORS=$$(( ( ${STAGE2_SIZE} + 511 ) / 512))
RESERVED_SECTORS=$$(( 1 + ${STAGE2_SECTORS} ))

STAGE1_FILE=$(BUILD_DIR)/bootloader/stage1.bin
STAGE1_MAP=$(BUILD_DIR)/bootloader/stage1.map
STAGE2_FILE=$(BUILD_DIR)/bootloader/stage2.elf

#
# Floppy Image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: objects
	@dd if=/dev/zero of=$@ bs=512 count=2880 >& /dev/null
	@mkfs.fat -F 12 -R ${RESERVED_SECTORS} -n "OS-DEVEL" $@ >& /dev/null
	@python build-tools/boot_data.py $@ ${STAGE1_FILE} ${STAGE1_MAP} ${STAGE2_FILE} 0
	@echo " > Copying kernel..."
	@mmd -i $@ "::boot"
	@mcopy -i $@ ${BUILD_DIR}/kernel/kernel.elf "::boot/kernel.elf"
	@echo " > Populating disk..."
	@python image/populate.py $@

run_floppy: floppy_image
	@qemu-system-i386 -m 32 -debugcon stdio -fda ${BUILD_DIR}/main_floppy.img

#
# Disk Image
#
DISK_SECTOR_COUNT=$$(( ( ${MAKE_DISK_SIZE} + 511 ) / 512 ))
DISK_PART1_BEGIN=2048
DISK_PART1_END=$$((${DISK_SECTOR_COUNT} - 1))

disk_image: $(BUILD_DIR)/main_disk.raw

$(BUILD_DIR)/main_disk.raw: objects $(BUILD_DIR)/main_disk.raw.tmp
	@echo "Creating main disk file $@ of size ${MAKE_DISK_SIZE} bytes"
	@dd if=/dev/zero of=$@ bs=512 count=${DISK_SECTOR_COUNT} >& /dev/null
	@echo " > Partitioning disk file..."
	@parted -s $@ mklabel msdos
	@parted -s $@ mkpart primary ${DISK_PART1_BEGIN}s ${DISK_PART1_END}s
	@parted -s $@ set 1 boot on
	@echo " > Copying main partition..."
	@dd if=$@.tmp of=$@ conv=notrunc bs=512 seek=${DISK_PART1_BEGIN} >& /dev/null
	@python build-tools/boot_data.py $@ ${STAGE1_FILE} ${STAGE1_MAP} ${STAGE2_FILE} ${DISK_PART1_BEGIN}
	@rm -f $@.tmp

$(BUILD_DIR)/main_disk.raw.tmp:
	@mkdir -p $(@D)
	@echo "Creating temporary disk file $@ of size $$(( (${DISK_SECTOR_COUNT} - ${DISK_PART1_BEGIN}) * 512 ))"
	@dd if=/dev/zero of=$@ bs=512 count=$$(( ${DISK_SECTOR_COUNT} - ${DISK_PART1_BEGIN} )) >& /dev/null
	@echo " > Formating to FAT${MAKE_PART_FORMAT}..."
	@mkfs.fat -F ${MAKE_PART_FORMAT} -n "OSDEVEL" $@ >& /dev/null
	@echo " > Copying kernel..."
	@mmd -i $@ "::boot"
	@mcopy -i $@ ${BUILD_DIR}/kernel/kernel.elf "::boot/kernel.elf"
	@echo " > Populating disk..."
	@python image/populate.py $@

run_disk: disk_image
	@qemu-system-i386 -m 32 -debugcon stdio -drive file=$(BUILD_DIR)/main_disk.raw,format=raw,index=0,media=disk

debug_disk: disk_image
	@gdb -x debug_disk.gdb

#
# Build objects
#
objects:
	@rm -rf ${BUILD_DIR}
	@rm -rf ${OBJ_DIR}
	@docker build --file Dockerfile --build-arg MAKE_CMD=all --output . .
