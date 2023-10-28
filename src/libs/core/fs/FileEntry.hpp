#pragma once

#include "File.hpp"

#include <stdint.h>

enum class FileType { File, Directory };
enum FileOpenMode { Read, Write, Append };

class FileEntry {
public:
  virtual const char *GetName() const = 0;
  virtual const FileType GetType() const = 0;

  virtual File *Open(FileOpenMode mode) = 0;
  virtual void Release() = 0;
};
