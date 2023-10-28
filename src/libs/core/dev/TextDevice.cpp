#include "TextDevice.hpp"

#include <stdint.h>

enum class PrintfState {
  Normal = 0,
  Length = 1,
  LengthShort = 2,
  LengthLong = 3,
  Spec = 4,
};

enum class PrintfLength {
  Default = 0,
  Short = 1,
  ShortShort = 2,
  Long = 3,
  LongLong = 4,
};

const char TextDevice::s_HexChars[] = "0123456789ABCDEF";

TextDevice::TextDevice(CharacterDevice *dev) : m_Device(dev) {}

bool TextDevice::Write(char c) {
  return m_Device->Write((uint8_t *)&c, sizeof(char)) == sizeof(char);
}

bool TextDevice::Write(const char *str) {
  bool ok = true;

  while (*str && ok) {
    ok = ok && Write(*str);
    str++;
  }

  return ok;
}

bool TextDevice::VFormat(const char *fmt, va_list args) {
  PrintfState state = PrintfState::Normal;
  PrintfLength length = PrintfLength::Default;
  int radix = 10;
  bool sign = true;
  bool number = false;
  bool ok = true;

  while (*fmt) {
    switch (state) {
    case PrintfState::Normal:
      switch (*fmt) {
      case '%':
        state = PrintfState::Length;
        break;
      default:
        ok = ok && Write(*fmt);
        break;
      }
      break;
    case PrintfState::Length:
      switch (*fmt) {
      case 'h':
        length = PrintfLength::Short;
        state = PrintfState::LengthShort;
        break;
      case 'l':
        length = PrintfLength::Long;
        state = PrintfState::LengthLong;
        break;
      default:
        goto PRINTF_STATE_SPEC_;
        break;
      }
      break;
    case PrintfState::LengthShort:
      if (*fmt == 'h') {
        length = PrintfLength::ShortShort;
        state = PrintfState::Spec;
      } else {
        goto PRINTF_STATE_SPEC_;
      }
      break;
    case PrintfState::LengthLong:
      if (*fmt == 'l') {
        length = PrintfLength::LongLong;
        state = PrintfState::Spec;
      } else {
        goto PRINTF_STATE_SPEC_;
      }
      break;
    case PrintfState::Spec:
    PRINTF_STATE_SPEC_:
      switch (*fmt) {
      case 'c':
        ok = ok && Write((char)va_arg(args, int));
        break;
      case 's':
        ok = ok && Write(va_arg(args, const char *));
        break;
      case '%':
        ok = ok && Write('%');
        break;
      case 'd':
      case 'i':
        radix = 10;
        sign = true;
        number = true;
        break;
      case 'u':
        radix = 10;
        sign = false;
        number = true;
        break;
      case 'X':
      case 'x':
      case 'p':
        radix = 16;
        sign = false;
        number = true;
        break;
      case 'o':
        radix = 8;
        sign = false;
        number = true;
        break;
        // Ignore invalid spec
      default:
        break;
      }

      if (number) {
        if (sign) {
          switch (length) {
          case PrintfLength::ShortShort:
          case PrintfLength::Short:
          case PrintfLength::Default:
            ok = ok && Write(va_arg(args, int), radix);
            break;
          case PrintfLength::Long:
            ok = ok && Write(va_arg(args, long), radix);
            break;
          case PrintfLength::LongLong:
            ok = ok && Write(va_arg(args, long long), radix);
            break;
          }
        } else {
          switch (length) {
          case PrintfLength::ShortShort:
          case PrintfLength::Short:
          case PrintfLength::Default:
            ok = ok && Write(va_arg(args, unsigned int), radix);
            break;
          case PrintfLength::Long:
            ok = ok && Write(va_arg(args, unsigned long), radix);
            break;
          case PrintfLength::LongLong:
            ok = ok && Write(va_arg(args, unsigned long long), radix);
            break;
          }
        }
      }

      state = PrintfState::Normal;
      length = PrintfLength::Default;
      radix = 10;
      sign = false;
      number = false;

      break;
    }

    fmt++;
  }

  return ok;
}

bool TextDevice::Format(const char *fmt, ...) {
  bool ok = true;

  va_list args;
  va_start(args, fmt);
  ok = ok && VFormat(fmt, args);
  va_end(args);

  return ok;
}

bool TextDevice::FormatBuffer(const char *msg, const void *buffer,
                              size_t size) {
  bool ok = true;
  ok = ok && Format(msg);

  for (int i = 0; i < size; i++) {
    if (i % 16 == 0)
      ok = ok && Format("\n%p:", i);

    if (i % 4 == 0)
      ok = ok && Format(" ");

    ok = ok && Format("%p ", ((uint8_t *)buffer)[i]);
  }

  ok = ok && Format("\n");

  return ok;
}
