#pragma once

#include "Defs.hpp"

#include <stdint.h>

namespace arch {
namespace i686 {

EXPORT void ASMCALL OutB(uint16_t port, uint8_t data);
EXPORT uint8_t ASMCALL InB(uint16_t port);

} // namespace i686
} // namespace arch
