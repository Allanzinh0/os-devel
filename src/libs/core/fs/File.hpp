#pragma once

#include "dev/BlockDevice.hpp"

#include <stddef.h>
#include <stdint.h>

enum class SeekPos { Set, Current, End };

class File : public BlockDevice {
public:
  virtual ~File() {}
};
