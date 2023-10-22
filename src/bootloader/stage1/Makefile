TARGET_ASMFLAGS += -f elf -DFILESYSTEM=${MAKE_PART_FORMAT}
TARGET_LINKFLAGS += -T linker.ld -nostdlib

SOURCES_ASM=$(wildcard *.asm)
OBJECTS_ASM=$(patsubst %.asm, $(OBJ_DIR)/stage1/asm/%.obj, $(SOURCES_ASM))

.PHONY: all clean

all: stage1

stage1: $(OBJ_DIR)/stage1.bin
	
$(OBJ_DIR)/stage1.bin: $(OBJECTS_ASM)
	$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/stage1.map -o $@ $^

$(OBJ_DIR)/stage1/asm/%.obj: %.asm always
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<

always:
	mkdir -p $(OBJ_DIR)/stage1/asm

clean:
	rm -f $(OBJ_DIR)/stage1.bin
	rm -rf $(OBJ_DIR)/stage1
