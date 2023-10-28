#pragma once

#include "FileSystem.hpp"

#include "fat/FATData.hpp"
#include "fat/FATFileEntry.hpp"

#include <core/dev/BlockDevice.hpp>

constexpr int FATRequiredMemory = 0x10000;

class FATFileSystem : public FileSystem {
public:
  FATFileSystem();
  virtual bool Initialize(BlockDevice *device) override;
  virtual File *RootDirectory() override;

  bool ReadSector(uint32_t lba, uint8_t *buffer, size_t count = 1);
  bool ReadSectorFromCluster(uint32_t cluster, uint32_t sectorOffset,
                             uint8_t *buffer);

  uint8_t GetFatType() const { return m_FatType; }
  const FATData &GetData() const { return *m_Data; }
  uint32_t GetNextCluster(uint32_t currentCluster);

  FATFile *AllocateFile();
  void ReleaseFile(FATFile *file);

  FATFileEntry *AllocateFileEntry();
  void ReleaseFileEntry(FATFileEntry *fileEntry);

private:
  bool ReadBootSector();
  uint32_t ClusterToLba(uint32_t cluster);
  uint8_t DetectFatType();
  bool ReadFat(uint32_t lbaOffset);

private:
  BlockDevice *m_Device;
  FATData *m_Data;
  uint32_t m_DataSectionLba;
  uint8_t m_FatType;
  uint32_t m_TotalSectors;
  uint32_t m_SectorsPerFat;
};
