#pragma once

#include <stdint.h>

void clrscr();
void putc(char c);
void puts(const char *str);
void printf(const char *fmt, ...);
void print_buffer(const char *fmt, const void *buffer, uint32_t count);

void putchr(int x, int y, char c);
