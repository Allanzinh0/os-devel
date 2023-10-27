#!/bin/bash
set -e

TARGET=$1
SIZE=$2

# Stage 2 sectors size
DISK_SECTOR_COUNT=$(( ( ${SIZE} + 511 ) / 512 ))
DISK_PART1_BEGIN=2048
STAGE2_SIZE=$( stat -c%s ${BUILD_DIR}/bootloader/stage2.bin )
STAGE2_SECTORS=$(( ( ${STAGE2_SIZE} + 511 ) / 512))

# Create disk image
dd if=/dev/zero of=$TARGET bs=512 count=$DISK_SECTOR_COUNT >/dev/null

# Create partition table
parted -s $TARGET mklabel msdos
parted -s $TARGET mkpart primary ${DISK_PART1_BEGIN}s $(( ${DISK_SECTOR_COUNT} - 1))s
parted -s $TARGET set 1 boot on

# Stage 2 bootloader
if [ ${STAGE2_SECTORS} \> $(( ${DISK_PART1_BEGIN} - 1 )) ]; then
  echo "Stage 2 too big for start of the disk"
  exit 2
fi

dd if=${BUILD_DIR}/bootloader/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1 >/dev/null

# Creating loopback partition
TARGET_PARTITION="${TARGET}.part1.raw"
dd if=/dev/zero of=$TARGET_PARTITION bs=512 count=$(( ${DISK_SECTOR_COUNT} - ${DISK_PART1_BEGIN} )) >/dev/null

mkfs.fat -F $MAKE_PART_FORMAT -n "OS-DEVEL" $TARGET_PARTITION >/dev/null

# Add bootloader
STAGE1_OFFSET=$(grep __entry_start ${BUILD_DIR}/bootloader/stage1.map | grep -oaE "0x[0-z]+")
STAGE1_OFFSET=$(( ${STAGE1_OFFSET} - 0x7c00 ))

dd if=${BUILD_DIR}/bootloader/stage1.bin of=$TARGET_PARTITION conv=notrunc bs=1 count=3 >/dev/null
dd if=${BUILD_DIR}/bootloader/stage1.bin of=$TARGET_PARTITION conv=notrunc bs=1 seek=$STAGE1_OFFSET skip=$STAGE1_OFFSET >/dev/null

# Write LBA address for stage 2 in bootloader
STAGE2_ENTRY=$(grep stage2_location ${BUILD_DIR}/bootloader/stage1.map | grep -oaE "0x[0-z]+")
STAGE2_ENTRY=$(( ${STAGE2_ENTRY} - 0x7c00 ))
echo "01 00 00 00" | xxd -r -p | dd of=$TARGET_PARTITION conv=notrunc bs=1 seek=$STAGE2_ENTRY >/dev/null
printf "%x" ${STAGE2_SECTORS} | xxd -r -p | dd of=$TARGET_PARTITION conv=notrunc bs=1 seek=$(( ${STAGE2_ENTRY} + 4 )) >/dev/null

# Add Files and kernel
mmd -i $TARGET_PARTITION "::boot"
mcopy -i $TARGET_PARTITION ${BUILD_DIR}/kernel/kernel.elf "::boot/kernel.elf"
mcopy -i $TARGET_PARTITION test.txt "::test.txt"
mmd -i $TARGET_PARTITION "::mydir"
mcopy -i $TARGET_PARTITION test.txt "::mydir/test.txt"

dd if=$TARGET_PARTITION of=$TARGET conv=notrunc bs=512 seek=$DISK_PART1_BEGIN >/dev/null

rm -f $TARGET_PARTITION
