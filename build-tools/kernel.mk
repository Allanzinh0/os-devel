KERNEL_SRC := ${SOURCE_DIR}/kernel

KERNEL_ASMFLAGS := ${TARGET_ASMFLAGS} -f elf -I$(KERNEL_SRC)
KERNEL_CFLAGS := ${TARGET_CFLAGS} -ffreestanding -nostdlib -I${KERNEL_SRC}
KERNEL_LIBS := ${TARGET_LIBS} -lgcc
KERNEL_LINKFLAGS := ${TARGET_LINKFLAGS} -T ${KERNEL_SRC}/linker.ld -nostdlib

# Assembly
KERNEL_SRC_ASM=$(shell find $(KERNEL_SRC) -type f -name  "*.asm")
KERNEL_HEADERS_ASM=$(shell find $(KERNEL_SRC) -type f -name  "*.inc")
KERNEL_OBJ_ASM=$(patsubst $(KERNEL_SRC)/%.asm, $(OBJ_DIR)/kernel/asm/%.obj, $(KERNEL_SRC_ASM))

# C
KERNEL_SRC_C=$(shell find $(KERNEL_SRC) -type f -name  "*.c")
KERNEL_HEADERS_C=$(shell find $(KERNEL_SRC) -type f -name  "*.h")
KERNEL_OBJ_C=$(patsubst $(KERNEL_SRC)/%.c, $(OBJ_DIR)/kernel/c/%.obj, $(KERNEL_SRC_C))

kernel: $(BUILD_DIR)/kernel/kernel.elf

$(BUILD_DIR)/kernel/kernel.elf: $(KERNEL_OBJ_ASM) $(KERNEL_OBJ_C)
	@mkdir -p $(@D)
	@$(TARGET_LD) $(KERNEL_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel/kernel.map -o $@ $^ $(KERNEL_LIBS)
	@echo " > Linked: $@ file"

$(OBJ_DIR)/kernel/c/%.obj: $(KERNEL_SRC)/%.c $(KERNEL_HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(KERNEL_CFLAGS) -c -o $@ $<
	@echo " > Builded: $@ file"

$(OBJ_DIR)/kernel/asm/%.obj: $(KERNEL_SRC)/%.asm $(KERNEL_HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(KERNEL_ASMFLAGS) -o $@ $<
	@echo " > Builded: $@ file"
