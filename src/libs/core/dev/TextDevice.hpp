#pragma once

#include "CharacterDevice.hpp"
#include "cpp/TypeTraits.hpp"

#include <stdarg.h>

class TextDevice {
public:
  TextDevice(CharacterDevice *dev);

  bool Write(char c);
  bool Write(const char *str);

  bool Format(const char *fmt, ...);
  bool FormatBuffer(const char *msg, const void *buffer, size_t size);

private:
  bool VFormat(const char *fmt, va_list args);

  template <typename TNumber> bool Write(TNumber num, int rad);

private:
  CharacterDevice *m_Device;
  static const char s_HexChars[];
};

template <typename TNumber> bool TextDevice::Write(TNumber number, int rad) {
  bool isSigned = IsSigned(number);
  TNumber num = number;

  char buffer[32];
  int pos = 0;

  if (isSigned && num < 0) {
    Write('-');
    num *= -1;
  }

  do {
    TNumber rem = num % rad;
    num /= rad;
    buffer[pos++] = s_HexChars[rem];
  } while (num > 0);

  bool ok = true;
  while (--pos >= 0)
    ok = ok && Write(buffer[pos]);

  return ok;
}
