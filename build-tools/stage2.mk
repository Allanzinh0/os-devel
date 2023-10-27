STAGE2_SRC := ${SOURCE_DIR}/bootloader/stage2

STAGE2_ASMFLAGS := ${TARGET_ASMFLAGS} -f elf
STAGE2_CFLAGS := ${TARGET_CFLAGS} -ffreestanding -nostdlib
STAGE2_LIBS := ${TARGET_LIBS} -lgcc
STAGE2_LINKFLAGS := ${TARGET_LINKFLAGS} -T ${STAGE2_SRC}/linker.ld -nostdlib

# Assembly
STAGE2_SRC_ASM=$(shell find $(STAGE2_SRC) -type f -name  "*.asm")
STAGE2_OBJ_ASM=$(patsubst $(STAGE2_SRC)/%.asm, $(OBJ_DIR)/stage2/asm/%.obj, $(STAGE2_SRC_ASM))

# C
STAGE2_SRC_C=$(shell find $(STAGE2_SRC) -type f -name  "*.c")
STAGE2_OBJ_C=$(patsubst $(STAGE2_SRC)/%.c, $(OBJ_DIR)/stage2/c/%.obj, $(STAGE2_SRC_C))

stage2: $(BUILD_DIR)/bootloader/stage2.bin
	
$(BUILD_DIR)/bootloader/stage2.bin: $(STAGE2_OBJ_ASM) $(STAGE2_OBJ_C)
	@mkdir -p $(@D)
	@$(TARGET_LD) $(STAGE2_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/bootloader/stage2.map -o $@ $^ $(STAGE2_LIBS)
	@echo " > Linked: $@ file"

$(OBJ_DIR)/stage2/asm/%.obj: $(STAGE2_SRC)/%.asm
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(STAGE2_ASMFLAGS) -o $@ $<
	@echo " > Builded: $@ file"

$(OBJ_DIR)/stage2/c/%.obj: $(STAGE2_SRC)/%.c
	@mkdir -p $(@D)
	@$(TARGET_CC) $(STAGE2_CFLAGS) -c -o $@ $<
	@echo " > Builded: $@ file"
