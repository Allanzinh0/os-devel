#pragma once

#include "FileEntry.hpp"

#include <core/dev/BlockDevice.hpp>

constexpr int MaxPathSize = 256;

class FileSystem {
public:
  virtual bool Initialize(BlockDevice *device) = 0;
  virtual File *RootDirectory() = 0;

  virtual File *Open(const char *path, FileOpenMode mode);

private:
  virtual FileEntry *FindFile(File *parentDir, const char *name);
};
