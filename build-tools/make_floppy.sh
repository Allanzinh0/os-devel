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
dd if=${OBJ_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 count=3 >/dev/null
dd if=${OBJ_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 seek=62 skip=62 >/dev/null
dd if=${OBJ_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1 >/dev/null

# Write LBA address for stage 2 in bootloader
echo "01 00 00 00" | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$STAGE1_STAGE2_LOCATION_OFFSET >/dev/null
printf "%x" ${STAGE2_SECTORS} | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$(( ${STAGE1_STAGE2_LOCATION_OFFSET} + 4 )) >/dev/null

# Add Files and kernel
mcopy -i $TARGET ${BUILD_DIR}/kernel.bin "::kernel.bin"
mcopy -i $TARGET test.txt "::test.txt"
mmd -i $TARGET "::mydir"
mcopy -i $TARGET test.txt "::mydir/test.txt"
