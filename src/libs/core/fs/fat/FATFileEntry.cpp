#include "FATFileEntry.hpp"
#include "core/Memory.hpp"

#include <core/Debug.hpp>
#include <core/fs/FATFileSystem.hpp>
#include <core/fs/fat/FATHeaders.hpp>

FATFileEntry::FATFileEntry() : m_Filesystem(nullptr), m_DirEntry() {}

void FATFileEntry::Initialize(FATFileSystem *fs,
                              const FATDirectoryEntry &dirEntry) {
  m_Filesystem = fs;
  m_DirEntry = dirEntry;

  // Set short name
  Memory::Set(m_ShortName, '\0', 12);

  uint32_t pos = 0;
  for (int i = 0; i < 8; i++) {
    if (m_DirEntry.Name[i] == ' ')
      break;

    m_ShortName[pos] = m_DirEntry.Name[i];
    pos++;
  }

  bool addedDot = false;
  for (int i = 8; i < 11; i++) {
    if (m_DirEntry.Name[i] == ' ')
      continue;

    if (!addedDot) {
      addedDot = true;
      m_ShortName[pos] = '.';
      pos++;
    }

    m_ShortName[pos] = m_DirEntry.Name[i];
    pos++;
  }
}

const char *FATFileEntry::GetName() const { return m_ShortName; }

const FileType FATFileEntry::GetType() const {
  if (m_DirEntry.Attributes & FAT_ATTRIBUTE_DIRECTORY)
    return FileType::Directory;

  return FileType::File;
}

File *FATFileEntry::Open(FileOpenMode mode) {
  FATFile *file = m_Filesystem->AllocateFile();

  if (file == nullptr) {
    return nullptr;
  }

  uint32_t size = m_DirEntry.Size;
  uint32_t firstCluster = m_DirEntry.FirstClusterLow +
                          ((uint32_t)m_DirEntry.FirstClusterHigh << 16);

  if (!file->Open(m_Filesystem, firstCluster, GetName(), size,
                  GetType() == FileType::Directory)) {
    Debug::Error("FATFileEntry", "Couldn't open the file %s", GetName());
    m_Filesystem->ReleaseFile(file);
    return nullptr;
  }

  return file;
}

void FATFileEntry::Release() { m_Filesystem->ReleaseFileEntry(this); }
