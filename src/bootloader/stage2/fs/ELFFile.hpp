#pragma once

// clang-format off
#define ELF_MAGIC ("\x7F""ELF")
// clang-format on

#include <core/fs/File.hpp>

class ELFFile {
public:
  bool Load(File *file);

  void *GetEntrypoint() const { return m_Entrypoint; }

private:
  void *m_Entrypoint;
};
