#pragma once

#include "dev/CharacterDevice.hpp"

#include <stddef.h>
#include <stdint.h>

namespace arch {
namespace i686 {

class E9Device : public CharacterDevice {
public:
  size_t Read(uint8_t *dataOut, size_t size);
  size_t Write(const uint8_t *data, size_t size);
};

} // namespace i686
} // namespace arch
