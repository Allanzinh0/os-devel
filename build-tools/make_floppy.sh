#!/bin/bash

TARGET=$1

# Stage 2 sectors size
STAGE2_SIZE=$( stat -c%s ${OBJ_DIR}/stage2.bin )
STAGE2_SECTORS=$(( ( ${STAGE2_SIZE} + 511 ) / 512))
RESERVED_SECTORS=$(( 1 + ${STAGE2_SECTORS} ))
STAGE1_STAGE2_LOCATION_OFFSET=480

# Create floppy image
dd if=/dev/zero of=$TARGET bs=512 count=2880 >/dev/null
mkfs.fat -F 12 -R ${RESERVED_SECTORS} -n "OS-DEVEL" $TARGET >/dev/null

# Add bootloader
STAGE1_OFFSET=$(grep __entry_start ${BUILD_DIR}/stage1.map | grep -oaE "0x[0-z]+")
STAGE1_OFFSET=$(( ${STAGE1_OFFSET} - 0x7c00 ))

dd if=${OBJ_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 count=3 >/dev/null
dd if=${OBJ_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 seek=$STAGE1_OFFSET skip=$STAGE1_OFFSET >/dev/null
dd if=${OBJ_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1 >/dev/null

# Write LBA address for stage 2 in bootloader
STAGE2_ENTRY=$(grep stage2_location ${BUILD_DIR}/stage1.map | grep -oaE "0x[0-z]+")
STAGE2_ENTRY=$(( ${STAGE2_ENTRY} - 0x7c00 ))
echo "01 00 00 00" | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$STAGE2_ENTRY >/dev/null
printf "%x" ${STAGE2_SECTORS} | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$(( ${STAGE2_ENTRY} + 4 )) >/dev/null

# Add Files and kernel
mmd -i $TARGET "::boot"
mcopy -i $TARGET ${BUILD_DIR}/kernel.bin "::boot/kernel.bin"
mcopy -i $TARGET test.txt "::test.txt"
mmd -i $TARGET "::mydir"
mcopy -i $TARGET test.txt "::mydir/test.txt"
