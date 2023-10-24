#pragma once

#include "mbr.h"

#include <stdbool.h>
#include <stdint.h>

#define ELF_MAGIC                                                              \
  ("\x7F"                                                                      \
   "ELF")

typedef struct {
  uint8_t Magic[4];
  uint8_t Bitness;
  uint8_t Endianness;
  uint8_t ELFHeaderVersion;
  uint8_t ABI;
  uint8_t _Padding[8];
  uint16_t Type;
  uint16_t InstructionSet;
  uint32_t ELFVersion;
  uint32_t ProgramEntryPosition;
  uint32_t ProgramHeaderTablePosition;
  uint32_t SectionHeadertablePosition;
  uint32_t Flags;
  uint16_t HeaderSize;
  uint16_t ProgramHeaderTableEntrySize;
  uint16_t ProgramHeaderTableEntryCount;
  uint16_t SectionHeaderTableEntrySize;
  uint16_t SectionHeaderTableEntryCount;
  uint16_t SectionNamesindex;
} __attribute__((packed)) ELFHeader;

enum ELFBitness {
  ELF_BITNESS_32BIT = 1,
  ELF_BITNESS_64BIT = 2,
};

enum ELFEndianness {
  ELF_ENDIANNESS_LITTLE = 1,
  ELF_ENDIANNESS_BIG = 2,
};

enum ELFInstructionSet {
  ELF_INSTRUCTION_SET_NONE = 0,
  ELF_INSTRUCTION_SET_X86 = 3,
  ELF_INSTRUCTION_SET_ARM = 0x28,
  ELF_INSTRUCTION_SET_X64 = 0x3E,
  ELF_INSTRUCTION_SET_ARM64 = 0xB7,
  ELF_INSTRUCTION_SET_RISCV = 0xF3,
};

enum ELFType {
  ELF_TYPE_RELOCATABLE = 1,
  ELF_TYPE_EXECUTABLE = 2,
  ELF_TYPE_SHARED = 3,
  ELF_TYPE_CORE = 4,
};

typedef struct {
  uint32_t Type;
  uint32_t Offset;
  uint32_t VirtualAddress;
  uint32_t PhysicalAddress;
  uint32_t FileSize;
  uint32_t MemorySize;
  uint32_t Flags;
  uint32_t Align;
} ELFProgramHeader;

enum ELFProgramType {
  ELF_PROGRAM_TYPE_NULL = 0x00000000,
  ELF_PROGRAM_TYPE_LOAD = 0x00000001,
  ELF_PROGRAM_TYPE_DYNAMIC = 0x00000002,
  ELF_PROGRAM_TYPE_INTERP = 0x00000003,
  ELF_PROGRAM_TYPE_NOTE = 0x00000004,
  ELF_PROGRAM_TYPE_SHLIB = 0x00000005,
  ELF_PROGRAM_TYPE_PHDR = 0x00000006,
  ELF_PROGRAM_TYPE_TLS = 0x00000007,
  ELF_PROGRAM_TYPE_LOOS = 0x60000000,
  ELF_PROGRAM_TYPE_HIOS = 0x6FFFFFFF,
  ELF_PROGRAM_TYPE_LOPROC = 0x70000000,
  ELF_PROGRAM_TYPE_HIPROC = 0x7FFFFFFF,
};

bool ELF_Open(Partition *part, const char *path, void **entrypoint);
