#pragma once

#include "Defs.hpp"

EXPORT const char *strchr(const char *str, char chr);
EXPORT char *strcpy(char *dst, const char *src);
EXPORT unsigned strlen(const char *str);
EXPORT int strcmp(const char *str1, const char *str2);

namespace String {
constexpr auto Find = strchr;
constexpr auto Copy = strcpy;
constexpr auto Length = strlen;
constexpr auto Compare = strcmp;

bool IsLower(char chr);
char ToUpper(char chr);
} // namespace String
