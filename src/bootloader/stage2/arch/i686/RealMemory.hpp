#pragma once

#include <stdint.h>

template <typename T> uint32_t ToSegOffset(T *addr) {
  uint32_t addrInt = reinterpret_cast<uint32_t>(addr);
  uint32_t seg = (addrInt >> 4) & 0xFFFF;
  uint32_t off = addrInt & 0xF;

  return (seg << 16) | off;
}

template <typename T> T ToLinear(uint32_t segOffset) {
  uint32_t off = (uint32_t)(segOffset)&0xFFFF;
  uint32_t seg = (uint32_t)(segOffset) >> 16;

  return (T)(seg * 16 + off);
}
