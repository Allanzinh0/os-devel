#include "FileSystem.hpp"
#include "core/Debug.hpp"
#include "core/Memory.hpp"
#include "core/String.hpp"

File *FileSystem::Open(const char *path, FileOpenMode mode) {
  char name[MaxPathSize];

  if (path[0] == '/')
    path++;

  File *root = RootDirectory();

  bool isLast = false;
  while (*path && !isLast) {
    const char *delim = String::Find(path, '/');

    if (delim != nullptr) {
      Memory::Copy(name, path, delim - path);
      name[delim - path] = '\0';
      path = delim + 1;
    } else {
      size_t len = String::Length(path);
      Memory::Copy(name, path, len);
      name[len + 1] = '\0';
      path += len;
      isLast = true;
    }

    FileEntry *nextEntry = FindFile(root, name);

    if (nextEntry != nullptr) {
      root->Release();

      // Check if directory
      if (!isLast && nextEntry->GetType() != FileType::Directory) {
        Debug::Warn("FileSystem", "%s is not a directory!", name);
        return nullptr;
      }

      root = nextEntry->Open(isLast ? mode : FileOpenMode::Read);
      nextEntry->Release();
    } else {
      root->Release();

      Debug::Warn("FileSystem", "%s not found!", name);
      return nullptr;
    }
  }

  return root;
}

FileEntry *FileSystem::FindFile(File *parentDir, const char *name) {
  FileEntry *entry = parentDir->ReadFileEntry();

  while (entry != nullptr) {
    if (String::Compare(entry->GetName(), name) == 0)
      return entry;

    entry->Release();
    entry = parentDir->ReadFileEntry();
  }

  return nullptr;
}

/*
 */
