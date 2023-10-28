STAGE2_SRC := ${SOURCE_DIR}/bootloader/stage2

STAGE2_ASMFLAGS := ${TARGET_ASMFLAGS} -f elf -I${STAGE2_SRC}
STAGE2_CFLAGS := ${TARGET_CFLAGS}
STAGE2_CXXFLAGS := ${TARGET_CXXFLAGS} -fno-exceptions -fno-rtti -I${SOURCE_DIR}/libs
STAGE2_LIBS := ${TARGET_LIBS} -lgcc -lcore
STAGE2_LINKFLAGS := ${TARGET_LINKFLAGS} -nostdlib -Wl,-T ${STAGE2_SRC}/linker.ld -L${BUILD_DIR}/libs

# Assembly
STAGE2_SRC_ASM=$(shell find $(STAGE2_SRC) -type f -name  "*.asm")
STAGE2_HEADERS_ASM=$(shell find $(STAGE2_SRC) -type f -name  "*.inc")
STAGE2_OBJ_ASM=$(patsubst $(STAGE2_SRC)/%.asm, $(OBJ_DIR)/stage2/asm/%.o, $(STAGE2_SRC_ASM))

# C
STAGE2_SRC_C=$(shell find $(STAGE2_SRC) -type f -name  "*.c")
STAGE2_HEADERS_C=$(shell find $(STAGE2_SRC) -type f -name  "*.h")
STAGE2_OBJ_C=$(patsubst $(STAGE2_SRC)/%.c, $(OBJ_DIR)/stage2/c/%.o, $(STAGE2_SRC_C))

# CPP
STAGE2_SRC_CXX=$(shell find $(STAGE2_SRC) -type f -name  "*.cpp")
STAGE2_HEADERS_CXX=$(shell find $(STAGE2_SRC) -type f -name  "*.hpp")
STAGE2_OBJ_CXX=$(patsubst $(STAGE2_SRC)/%.cpp, $(OBJ_DIR)/stage2/cpp/%.o, $(STAGE2_SRC_CXX))

# GCC objects
CRTI_OBJ=$(shell echo "$(STAGE2_OBJ_ASM)" | grep -oaE "[0-z/]+crti.o")
CRTBEGIN_OBJ=$(shell find ./toolchain/gcc-build/i686-elf/libgcc -type f -name "crtbegin.o")
CRTEND_OBJ=$(shell find ./toolchain/gcc-build/i686-elf -type f -name "crtend.o")
CRTN_OBJ=$(shell echo "$(STAGE2_OBJ_ASM)" | grep -oaE "[0-z/]+crtn.o")

stage2: $(BUILD_DIR)/bootloader/stage2.elf
	
$(BUILD_DIR)/bootloader/stage2.elf: $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(STAGE2_OBJ_ASM) $(STAGE2_OBJ_CXX) $(CRTEND_OBJ) $(CRTN_OBJ)
	@mkdir -p $(@D)
	@$(TARGET_LD) -Wl,-Map=$(BUILD_DIR)/bootloader/stage2.map -o $@ $^ $(STAGE2_LIBS) $(STAGE2_LINKFLAGS)
	@echo $$(stat -c%s $@)
	@echo " > Linked: $@ file"

$(OBJ_DIR)/stage2/asm/%.o: $(STAGE2_SRC)/%.asm $(STAGE2_HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(STAGE2_ASMFLAGS) -o $@ $<
	@echo " > Builded: $@ file"

$(OBJ_DIR)/stage2/c/%.o: $(STAGE2_SRC)/%.c $(STAGE2_HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(STAGE2_CFLAGS) -c -o $@ $<
	@echo " > Builded: $@ file"

$(OBJ_DIR)/stage2/cpp/%.o: $(STAGE2_SRC)/%.cpp $(STAGE2_HEADERS_CXX)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(STAGE2_CXXFLAGS) -c -o $@ $<
	@echo " > Builded: $@ file"
