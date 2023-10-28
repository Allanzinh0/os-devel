#pragma once

#include "CharacterDevice.hpp"

#include <stddef.h>
#include <stdint.h>

enum class SeekPos { Set, Current, End };

class BlockDevice : public CharacterDevice {
public:
  virtual ~BlockDevice() {}

  virtual bool Seek(SeekPos pos, int rel) = 0;
  virtual size_t Size() = 0;
  virtual size_t Position() = 0;
};
