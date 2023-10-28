#pragma once

#include "core/fs/fat/FATHeaders.hpp"
#include <core/fs/FileEntry.hpp>

class FATFileSystem;

class FATFileEntry : public FileEntry {
public:
  FATFileEntry();
  void Initialize(FATFileSystem *fs, const FATDirectoryEntry &dirEntry);
  virtual const char *GetName() const override;
  virtual const FileType GetType() const override;

  virtual File *Open(FileOpenMode mode) override;
  virtual void Release() override;

private:
  FATFileSystem *m_Filesystem;
  FATDirectoryEntry m_DirEntry;
  char m_ShortName[12];
};
