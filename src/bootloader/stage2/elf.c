#include "elf.h"

#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "minmax.h"
#include "stdio.h"

bool ELF_Open(Partition *part, const char *path, void **entrypoint) {
  uint8_t *headerBuffer = MEMORY_ELF_ADDR;
  uint8_t *loadBuffer = MEMORY_LOAD_KERNEL;
  uint32_t filePos = 0;
  uint32_t read;

  // Read header
  FAT_File *fd = FAT_Open(part, path);
  if ((read = FAT_Read(part, fd, sizeof(ELFHeader), headerBuffer)) !=
      sizeof(ELFHeader)) {
    printf("[ ELF  ]: ELF Header load error!\n");
    return false;
  }

  filePos += read;

  // Validate Header
  bool ok = true;
  ELFHeader *header = (ELFHeader *)headerBuffer;
  ok = ok && (memcmp(header->Magic, ELF_MAGIC, 4) == 0);
  ok = ok && (header->Bitness == ELF_BITNESS_32BIT);
  ok = ok && (header->Endianness == ELF_ENDIANNESS_LITTLE);
  ok = ok && (header->ELFHeaderVersion == 1);
  ok = ok && (header->ELFVersion == 1);
  ok = ok && (header->Type == ELF_TYPE_EXECUTABLE);
  ok = ok && (header->InstructionSet == ELF_INSTRUCTION_SET_X86);

  *entrypoint = (void *)(header->ProgramEntryPosition);

  // Load program header
  uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
  uint32_t programHeaderSize = header->ProgramHeaderTableEntrySize *
                               header->ProgramHeaderTableEntryCount;
  uint32_t programHeaderTableEntrySize = header->ProgramHeaderTableEntrySize;
  uint32_t programHeaderTableEntryCount = header->ProgramHeaderTableEntryCount;

  filePos += FAT_Read(part, fd, programHeaderOffset - filePos, headerBuffer);
  if ((read = FAT_Read(part, fd, programHeaderSize, headerBuffer)) !=
      programHeaderSize) {
    printf("[ ELF  ]: ELF Program header load error!\n");
    return false;
  }
  filePos += read;
  FAT_Close(fd);

  // Parse program header entries
  for (uint32_t i = 0; i < programHeaderTableEntryCount; i++) {
    ELFProgramHeader *progHeader =
        (ELFProgramHeader *)(headerBuffer + i * programHeaderTableEntrySize);

    if (progHeader->Type == ELF_PROGRAM_TYPE_LOAD) {
      // TODO: Validate that the program doesn't overwrite
      uint8_t *virtAddress = (uint8_t *)(progHeader->VirtualAddress);
      memset(virtAddress, 0, progHeader->MemorySize);

      // TODO: Proper seeking
      fd = FAT_Open(part, path);

      while (progHeader->Offset > 0) {
        uint32_t shouldRead = min(progHeader->Offset, MEMORY_LOAD_SIZE);
        read = FAT_Read(part, fd, shouldRead, loadBuffer);

        if (read != shouldRead) {
          printf("[ ELF  ]: ELF Program header seeking error!\n");
          return false;
        }

        progHeader->Offset -= read;
      }

      // Read Program
      while (progHeader->FileSize > 0) {
        uint32_t shouldRead = min(progHeader->FileSize, MEMORY_LOAD_SIZE);
        read = FAT_Read(part, fd, shouldRead, loadBuffer);

        if (read != shouldRead) {
          printf("[ ELF  ]: ELF Program header reading error!\n");
          return false;
        }

        progHeader->FileSize -= read;

        memcpy(virtAddress, loadBuffer, read);
        virtAddress += read;
      }
      FAT_Close(fd);
    }
  }

  return ok;
}
