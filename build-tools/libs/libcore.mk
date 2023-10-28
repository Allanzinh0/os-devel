LIBCORE_SRC := ${SOURCE_DIR}/libs/core

LIBCORE_ASMFLAGS := ${TARGET_ASMFLAGS} -f elf
LIBCORE_CFLAGS := ${TARGET_CFLAGS} -ffreestanding -nostdlib
LIBCORE_CXXFLAGS := ${TARGET_CXXFLAGS} -ffreestanding -fno-exceptions -fno-rtti -I${SOURCE_DIR}/libs -I$(LIBCORE_SRC)
LIBCORE_LIBS := ${TARGET_LIBS}

# Assembly
LIBCORE_SRC_ASM=$(shell find $(LIBCORE_SRC) -type f -name "*.asm")
LIBCORE_HEADERS_ASM=$(shell find $(LIBCORE_SRC) -type f -name "*.inc")
LIBCORE_OBJ_ASM=$(patsubst $(LIBCORE_SRC)/%.asm, $(OBJ_DIR)/libs/libcore/asm/%.obj, $(LIBCORE_SRC_ASM))

# C
LIBCORE_SRC_C=$(shell find $(LIBCORE_SRC) -type f -name "*.c")
LIBCORE_HEADERS_C=$(shell find $(LIBCORE_SRC) -type f -name "*.h")
LIBCORE_OBJ_C=$(patsubst $(LIBCORE_SRC)/%.c, $(OBJ_DIR)/libs/libcore/c/%.obj, $(LIBCORE_SRC_C))

# CPP
LIBCORE_SRC_CXX=$(shell find $(LIBCORE_SRC) -type f -name "*.cpp")
LIBCORE_HEADERS_CXX=$(shell find $(LIBCORE_SRC) -type f -name "*.hpp")
LIBCORE_OBJ_CXX=$(patsubst $(LIBCORE_SRC)/%.cpp, $(OBJ_DIR)/libs/libcore/cpp/%.obj, $(LIBCORE_SRC_CXX))

$(BUILD_DIR)/libs/libcore.a: $(LIBCORE_OBJ_ASM) $(LIBCORE_OBJ_C) $(LIBCORE_OBJ_CXX)
	@mkdir -p $(@D)
	@$(TARGET_AR) rcs $@ $^ $(LIBCORE_LIBS)
	@echo " > Static Library: $@ file"

$(OBJ_DIR)/libs/libcore/asm/%.obj: $(LIBCORE_SRC)/%.asm $(LIBCORE_HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(LIBCORE_ASMFLAGS) -o $@ $<
	@echo " > Builded: $@ file"

$(OBJ_DIR)/libs/libcore/c/%.obj: $(LIBCORE_SRC)/%.c $(LIBCORE_HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(LIBCORE_CFLAGS) -c -o $@ $<
	@echo " > Builded: $@ file"

$(OBJ_DIR)/libs/libcore/cpp/%.obj: $(LIBCORE_SRC)/%.cpp $(LIBCORE_HEADERS_CXX)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(LIBCORE_CXXFLAGS) -c -o $@ $<
	@echo " > Builded: $@ file"
