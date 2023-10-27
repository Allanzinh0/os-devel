STAGE1_SRC := ${SOURCE_DIR}/bootloader/stage1

STAGE1_ASMFLAGS := ${TARGET_ASMFLAGS} -f elf -DFILESYSTEM=${MAKE_PART_FORMAT}
STAGE1_LINKFLAGS := ${TARGET_LINKFLAGS} -T ${STAGE1_SRC}/linker.ld -nostdlib

STAGE1_SRC_ASM=$(shell find $(STAGE1_SRC) -type f -name  "*.asm")
STAGE1_OBJ_ASM=$(patsubst $(STAGE1_SRC)/%.asm, $(OBJ_DIR)/stage1/asm/%.obj, $(STAGE1_SRC_ASM))

stage1: $(BUILD_DIR)/bootloader/stage1.bin
	
$(BUILD_DIR)/bootloader/stage1.bin: $(STAGE1_OBJ_ASM)
	@mkdir -p $(@D)
	@$(TARGET_LD) $(STAGE1_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/bootloader/stage1.map -o $@ $^
	@echo " > Linked: $@ file"

$(OBJ_DIR)/stage1/asm/%.obj: $(STAGE1_SRC)/%.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(STAGE1_ASMFLAGS) -o $@ $<
	@echo " > Builded: $@ file"
