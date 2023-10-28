include build-tools/config.mk
include build-tools/image.mk

.PHONY: all floppy_image kernel clean run docker_floppy docker_disk

OS_LIBS=$(BUILD_DIR)/libs/libcore.a
all: $(OS_LIBS) kernel stage1 stage2

include build-tools/stage1.mk
include build-tools/stage2.mk
include build-tools/kernel.mk

#
# LIBRARYS
#
include build-tools/libs/libcore.mk

#
# Clean
#
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OBJ_DIR)
