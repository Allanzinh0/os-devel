#pragma once

#include "../cpp/TypeTraits.hpp"
#include "CharacterDevice.hpp"

#include <stdarg.h>

class TextDevice {
public:
  TextDevice(CharacterDevice *dev);
  virtual ~TextDevice() {}

  bool Write(char c);
  bool Write(const char *str);

  bool Format(const char *fmt, ...);
  bool VFormat(const char *fmt, va_list args);
  bool FormatBuffer(const char *msg, const void *buffer, size_t size);

  template <typename TNumber> bool WriteNumber(TNumber num, int rad);

  CharacterDevice *m_Device;

private:
  static const char s_HexChars[];
};

template <typename TNumber>
bool TextDevice::WriteNumber(TNumber number, int rad) {
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
