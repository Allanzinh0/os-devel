#include "FATFile.hpp"
#include "core/Memory.hpp"
#include "core/String.hpp"
#include "core/fs/fat/FATHeaders.hpp"

#include <core/Debug.hpp>
#include <core/cpp/Algorithm.hpp>
#include <core/fs/FATFileSystem.hpp>

FATFile::FATFile()
    : m_Filesystem(nullptr), m_Opened(false), m_IsRootDirectory(false),
      m_IsDirectory(false), m_FirstCluster(0), m_CurrentCluster(0),
      m_CurrentSectorInCluster(0), m_Position(0), m_Size(0),
      m_CurrentClusterIndex(0) {}

bool FATFile::Open(FATFileSystem *fs, uint32_t firstCluster, const char *name,
                   uint32_t size, bool isDirectory) {
  m_Filesystem = fs;
  m_Position = 0;
  m_Size = size;
  m_FirstCluster = firstCluster;
  m_CurrentCluster = firstCluster;
  m_CurrentSectorInCluster = 0;
  m_CurrentClusterIndex = 0;
  m_IsDirectory = isDirectory;

  if (!m_Filesystem->ReadSectorFromCluster(
          m_CurrentCluster, m_CurrentSectorInCluster, m_Buffer)) {
    Debug::Error("FatFile", "Read entry failed! CurrentCluster:%u",
                 m_CurrentCluster);
    return false;
  }

  return true;
}

bool FATFile::OpenRootDirectory1216(FATFileSystem *fs, uint32_t lba,
                                    uint32_t size) {
  m_Filesystem = fs;
  m_IsRootDirectory = true;
  m_IsDirectory = true;

  FATData data = m_Filesystem->GetData();

  m_Position = 0;
  m_Size = size;
  m_FirstCluster = lba;
  m_CurrentCluster = m_FirstCluster;
  m_CurrentSectorInCluster = 0;
  m_CurrentClusterIndex = 0;

  if (!m_Filesystem->ReadSector(lba, m_Buffer)) {
    Debug::Error("Fat", "Read root directory failed!");
    return false;
  }

  return true;
}

bool FATFile::Seek(SeekPos pos, int rel) {
  switch (pos) {
  case SeekPos::Set:
    m_Position = (uint32_t)Max(0, rel);
    break;
  case SeekPos::Current:
    if (rel < 0 && -rel > m_Position)
      m_Position = 0;
    else
      m_Position = (uint32_t)Min(m_Size, m_Position + rel);
    break;
  case SeekPos::End:
    if (rel < 0 && -rel > Size())
      m_Position = 0;
    else
      m_Position = (uint32_t)Min(m_Size, m_Position + rel);
    break;
  }

  return UpdateCurrentCluster();
}

size_t FATFile::Read(uint8_t *dataOut, size_t count) {
  uint32_t initialPosition = m_Position;
  // Don't read past end the end of file
  if (!m_IsDirectory || (m_IsDirectory && m_Size != 0))
    count = Min((uint32_t)count, m_Size - m_Position);

  while (count > 0) {
    uint32_t leftInBuffer = SectorSize - (m_Position % SectorSize);
    uint32_t take = Min((uint32_t)count, leftInBuffer);

    Memory::Copy(dataOut, m_Buffer + (m_Position % SectorSize), take);

    dataOut += take;
    m_Position += take;
    count -= take;

    if (leftInBuffer == take) {
      if (m_IsRootDirectory) {
        ++m_CurrentCluster;

        if (!m_Filesystem->ReadSector(m_CurrentCluster, m_Buffer)) {
          Debug::Error("FatFile", "Read next sector of root directory failed!");
          break;
        }
      } else {
        if (++m_CurrentSectorInCluster >=
            m_Filesystem->GetData().BS.BootSector.SectorsPerCluster) {
          m_CurrentSectorInCluster = 0;
          m_CurrentCluster = m_Filesystem->GetNextCluster(m_CurrentCluster);
          ++m_CurrentClusterIndex;
        }

        if (m_CurrentCluster >= 0xFFFFFFF8) {
          m_Size = m_Position;
          break;
        }

        if (!m_Filesystem->ReadSectorFromCluster(
                m_CurrentCluster, m_CurrentSectorInCluster, m_Buffer)) {
          Debug::Error("FatFile", "Read next sector of entry failed!");
          break;
        }
      }
    }
  }

  return m_Position - initialPosition;
}

size_t FATFile::Write(const uint8_t *data, size_t size) { return 0; }

bool FATFile::ReadFileEntry(FATDirectoryEntry *dirEntry) {
  bool ok = Read((uint8_t *)(dirEntry), sizeof(FATDirectoryEntry)) ==
            sizeof(FATDirectoryEntry);

  if (ok && dirEntry->Name[0] == '\0')
    return false;

  return ok;
}

bool FATFile::UpdateCurrentCluster() {
  uint32_t clusterSize =
      m_Filesystem->GetData().BS.BootSector.SectorsPerCluster * SectorSize;
  uint32_t desiredCluster = m_Position / clusterSize;
  uint32_t desiredSector = (m_Position % clusterSize) / SectorSize;

  if (desiredCluster == m_CurrentClusterIndex &&
      desiredSector == m_CurrentSectorInCluster)
    return true;

  if (desiredCluster < m_CurrentClusterIndex) {
    m_CurrentClusterIndex = 0;
    m_CurrentCluster = m_FirstCluster;
  }

  while (desiredCluster > m_CurrentClusterIndex) {
    m_CurrentCluster = m_Filesystem->GetNextCluster(m_CurrentCluster);
    ++m_CurrentClusterIndex;
  }

  m_CurrentSectorInCluster = desiredSector;
  return m_Filesystem->ReadSectorFromCluster(
      m_CurrentCluster, m_CurrentSectorInCluster, m_Buffer);
}

FileEntry *FATFile::ReadFileEntry() {
  FATDirectoryEntry entry;
  if (ReadFileEntry(&entry)) {
    FATFileEntry *fileEntry = m_Filesystem->AllocateFileEntry();
    fileEntry->Initialize(m_Filesystem, entry);

    return fileEntry;
  }

  return nullptr;
}

void FATFile::Release() { m_Filesystem->ReleaseFile(this); }
