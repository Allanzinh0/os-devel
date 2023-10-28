#include "FATFileSystem.hpp"

#include "fat/FATData.hpp"
#include "fat/FATHeaders.hpp"

#include <core/Debug.hpp>

FATFileSystem::FATFileSystem()
    : m_Device(), m_Data(new FATData()), m_DataSectionLba(), m_FatType(),
      m_TotalSectors(), m_SectorsPerFat() {}

bool FATFileSystem::Initialize(BlockDevice *device) {
  m_Device = device;

  Debug::Info("Fat", "Reading boot sector.");
  if (!ReadBootSector()) {
    Debug::Error("Fat", "Read boot sector failed!");
    return false;
  }

  m_Data->FatCachePosition = 0xFFFFFFFF;

  Debug::Info("Fat", "Reading FAT.");
  m_TotalSectors = m_Data->BS.BootSector.TotalSectors;
  if (m_TotalSectors == 0) {
    m_FatType = 32;
    m_TotalSectors = m_Data->BS.BootSector.LargeSectorCount;
  }

  m_SectorsPerFat = m_Data->BS.BootSector.SectorsPerFat;
  if (m_SectorsPerFat == 0) {
    m_FatType = 32;
    m_SectorsPerFat = m_Data->BS.BootSector.EBR32.SectorsPerFat;
  }

  uint32_t rootDirLba;
  uint32_t rootDirSize;

  Debug::Info("Fat", "Reading root directory.");
  if (m_FatType == 32) {
    m_DataSectionLba = m_Data->BS.BootSector.ReservedSectors +
                       m_SectorsPerFat * m_Data->BS.BootSector.FatCount;

    if (!m_Data->RootDirectory.Open(
            this, m_Data->BS.BootSector.EBR32.RootDirCluster, "", 0, true)) {
      Debug::Error("Fat", "Couldn't open the root directory.");
      return false;
    }
  } else {
    rootDirLba = m_Data->BS.BootSector.ReservedSectors +
                 m_SectorsPerFat * m_Data->BS.BootSector.FatCount;
    rootDirSize =
        sizeof(FATDirectoryEntry) * m_Data->BS.BootSector.DirEntriesCount;
    uint32_t rootDirSectors =
        (rootDirSize + m_Data->BS.BootSector.BytesPerSector - 1) /
        m_Data->BS.BootSector.BytesPerSector;
    m_DataSectionLba = rootDirLba + rootDirSectors;

    if (!m_Data->RootDirectory.OpenRootDirectory1216(this, rootDirLba,
                                                     rootDirSize)) {
      Debug::Error("Fat", "Couldn't open the root directory.");
      return false;
    }
  }

  m_FatType = DetectFatType();
  Debug::Info("Fat", "Current filesystem FAT%u.", m_FatType);

  return true;
}

FATFile *FATFileSystem::AllocateFile() {
  return m_Data->OpenedFilePool.Allocate();
}

void FATFileSystem::ReleaseFile(FATFile *file) {
  m_Data->OpenedFilePool.Free(file);
}

FATFileEntry *FATFileSystem::AllocateFileEntry() {
  return m_Data->FileEntryPool.Allocate();
}

void FATFileSystem::ReleaseFileEntry(FATFileEntry *fileEntry) {
  m_Data->FileEntryPool.Free(fileEntry);
}

File *FATFileSystem::RootDirectory() { return &m_Data->RootDirectory; }

bool FATFileSystem::ReadBootSector() {
  return ReadSector(0, (uint8_t *)&m_Data->BS.BootSectorBytes);
}

bool FATFileSystem::ReadSector(uint32_t lba, uint8_t *buffer, size_t count) {
  m_Device->Seek(SeekPos::Set, lba * SectorSize);
  return m_Device->Read(buffer, count * SectorSize) == count * SectorSize;
}

bool FATFileSystem::ReadSectorFromCluster(uint32_t cluster,
                                          uint32_t sectorOffset,
                                          uint8_t *buffer) {
  return ReadSector(ClusterToLba(cluster) + sectorOffset, buffer);
}

uint32_t FATFileSystem::ClusterToLba(uint32_t cluster) {
  return m_DataSectionLba +
         (cluster - 2) * m_Data->BS.BootSector.SectorsPerCluster;
}

uint8_t FATFileSystem::DetectFatType() {
  uint32_t dataClusters = (m_TotalSectors - m_DataSectionLba) /
                          m_Data->BS.BootSector.SectorsPerCluster;

  if (dataClusters < 0xFF5)
    return 12;

  if (m_Data->BS.BootSector.SectorsPerFat != 0)
    return 16;

  return 32;
}

uint32_t FATFileSystem::GetNextCluster(uint32_t currentCluster) {
  // Determine the byte offset
  uint32_t fatIndex;
  if (m_FatType == 12) {
    fatIndex = currentCluster * 3 / 2;
  } else if (m_FatType == 16) {
    fatIndex = currentCluster * 2;
  } else if (m_FatType == 32) {
    fatIndex = currentCluster * 4;
  }

  // Cache has the right number
  uint32_t fatIndexSector = fatIndex / SectorSize;
  if (fatIndexSector < m_Data->FatCachePosition ||
      fatIndexSector >= m_Data->FatCachePosition + FatCacheSize) {
    ReadFat(fatIndexSector);
  }

  fatIndex -= (m_Data->FatCachePosition * SectorSize);

  uint32_t nextCluster;

  if (m_FatType == 12) {
    if (currentCluster % 2 == 0)
      nextCluster = (*(uint16_t *)(m_Data->FatCache + fatIndex)) & 0x0FFF;
    else
      nextCluster = (*(uint16_t *)(m_Data->FatCache + fatIndex)) >> 4;

    if (nextCluster >= 0xFF8)
      nextCluster |= 0xFFFFF000;

    return nextCluster;
  }
  if (m_FatType == 16) {
    nextCluster = (*(uint16_t *)(m_Data->FatCache + fatIndex));

    if (nextCluster >= 0xFFF8)
      nextCluster |= 0xFFFF0000;

    return nextCluster;
  }

  // if (g_FatType == 32)
  return (*(uint32_t *)(m_Data->FatCache + fatIndex));
}

bool FATFileSystem::ReadFat(uint32_t lbaOffset) {
  m_Data->FatCachePosition = lbaOffset;
  return ReadSector(m_Data->BS.BootSector.ReservedSectors + lbaOffset,
                    m_Data->FatCache, FatCacheSize);
}
