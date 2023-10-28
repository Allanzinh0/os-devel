#include "E9Device.hpp"

#include "IO.h"

namespace arch {
namespace i686 {

size_t E9Device::Read(uint8_t *dataOut, size_t size) { return 0; }
size_t E9Device::Write(const uint8_t *data, size_t size) {
  for (int i = 0; i < size; i++)
    OutB(0xE9, data[i]);

  return size;
}

} // namespace i686
} // namespace arch
