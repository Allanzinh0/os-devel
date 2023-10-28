#include "String.hpp"

const char *strchr(const char *str, char chr) {
  if (str == nullptr)
    return nullptr;

  while (*str) {
    if (*str == chr)
      return str;

    ++str;
  }

  return nullptr;
}

char *strcpy(char *dst, const char *src) {
  char *origDst = dst;

  if (dst == nullptr)
    return nullptr;

  if (src == nullptr) {
    *dst = '\0';
    return dst;
  }

  while (*src) {
    *dst = *src;
    ++dst;
    ++src;
  }

  *dst = '\0';

  return origDst;
}

unsigned strlen(const char *str) {
  unsigned len = 0;

  while (*str) {
    ++len;
    ++str;
  }

  return len;
}

int strcmp(const char *str1, const char *str2) {
  if (str1 == nullptr && str2 == nullptr)
    return 0;

  if (str1 == nullptr || str2 == nullptr)
    return -1;

  while (*str1 && *str2 && *str1 == *str2) {
    ++str1;
    ++str2;
  }

  return (*str1) - (*str2);
}

bool String::IsLower(char chr) { return chr >= 'a' && chr <= 'z'; }

char String::ToUpper(char chr) {
  return IsLower(chr) ? (chr - 'a' + 'A') : chr;
}
