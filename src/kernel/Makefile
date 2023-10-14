TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib
TARGET_LIBS += -lgcc
TARGET_LINKFLAGS += -T linker.ld -nostdlib

SOURCES_C=$(wildcard *.c)
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(OBJ_DIR)/kernel/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(OBJ_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

.PHONY: all kernel clean always

all: clean kernel

kernel: $(BUILD_DIR)/kernel.bin
	
$(BUILD_DIR)/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS)

$(OBJ_DIR)/kernel/c/%.obj: %.c always
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

$(OBJ_DIR)/kernel/asm/%.obj: %.asm always
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<

always:
	mkdir -p $(OBJ_DIR)/kernel/c
	mkdir -p $(OBJ_DIR)/kernel/asm

clean:
	rm -f $(OBJ_DIR)/kernel.bin
