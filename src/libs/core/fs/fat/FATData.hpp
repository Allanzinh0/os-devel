#pragma once

#include "FATFile.hpp"
#include "FATFileEntry.hpp"
#include "FATHeaders.hpp"

#include <core/memory/StaticObjectPool.hpp>

#include <stdint.h>

constexpr int MaxFileHandles = 10;
constexpr int RootDirectoryHandle = -1;
constexpr int FatCacheSize = 5;
constexpr uint32_t FATLFNLast = 0x40;

struct FATData {
  union {
    FATBootSector BootSector;
    uint8_t BootSectorBytes[SectorSize];
  } BS;

  FATFile RootDirectory;
  StaticObjectPool<FATFile, MaxFileHandles> OpenedFilePool;
  StaticObjectPool<FATFileEntry, MaxFileHandles> FileEntryPool;

  uint8_t FatCache[FatCacheSize * SectorSize];
  uint32_t FatCachePosition;
};
