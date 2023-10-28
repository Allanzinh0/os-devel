#include "ELFFile.hpp"
#include "core/Debug.hpp"
#include "core/Memory.hpp"
#include "core/String.hpp"

#include <stdint.h>

struct ELFHeader {
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
} __attribute__((packed));

struct ELFProgramHeader {
  uint32_t Type;
  uint32_t Offset;
  uint32_t VirtualAddress;
  uint32_t PhysicalAddress;
  uint32_t FileSize;
  uint32_t MemorySize;
  uint32_t Flags;
  uint32_t Align;
} __attribute__((packed));

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

bool ELFFile::Load(File *file) {
  uint8_t elfHeader[sizeof(ELFHeader)];

  file->Seek(SeekPos::Set, 0);
  size_t readed = file->Read(elfHeader, sizeof(ELFHeader));
  if (readed != sizeof(ELFHeader)) {
    Debug::Error("ELFFile", "ELF file could not be readed.");
    return false;
  }

  ELFHeader *header = (ELFHeader *)elfHeader;
  bool ok = true;
  ok = ok && String::Compare((const char *)header->Magic, ELF_MAGIC) ==
                 header->Bitness;
  ok = ok && (header->Bitness == ELF_BITNESS_32BIT);
  ok = ok && (header->Endianness == ELF_ENDIANNESS_LITTLE);
  ok = ok && (header->ELFHeaderVersion == 1);
  ok = ok && (header->ELFVersion == 1);
  ok = ok && (header->Type == ELF_TYPE_EXECUTABLE);
  ok = ok && (header->InstructionSet == ELF_INSTRUCTION_SET_X86);

  if (!ok) {
    Debug::Error("ELFFile", "ELF format not supported.");
    return false;
  }

  m_Entrypoint = (void *)header->ProgramEntryPosition;

  // Reading program header
  uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
  uint32_t programHeaderSize = header->ProgramHeaderTableEntrySize *
                               header->ProgramHeaderTableEntryCount;
  uint32_t programHeaderTableEntrySize = header->ProgramHeaderTableEntrySize;
  uint32_t programHeaderTableEntryCount = header->ProgramHeaderTableEntryCount;

  uint8_t programHeader[programHeaderSize];
  file->Seek(SeekPos::Set, programHeaderOffset);
  readed = file->Read(programHeader, programHeaderSize);
  if (readed != programHeaderSize) {
    Debug::Error("ELFFile", "ELF program header could not be loaded.");
    return false;
  }

  for (int i = 0; i < programHeaderTableEntryCount; i++) {
    ELFProgramHeader *progHeader =
        (ELFProgramHeader *)(programHeader + i * programHeaderTableEntrySize);

    if (progHeader->Type == ELF_PROGRAM_TYPE_LOAD) {
      uint8_t *virtAddr = (uint8_t *)progHeader->VirtualAddress;
      Memory::Set(virtAddr, '\0', progHeader->MemorySize);

      file->Seek(SeekPos::Set, progHeader->Offset);
      if (file->Read(virtAddr, progHeader->MemorySize) !=
          progHeader->MemorySize) {
        Debug::Error("ELFFile", "ELF program could not be loaded.");
        return false;
      }
    }
  }

  return true;
}
