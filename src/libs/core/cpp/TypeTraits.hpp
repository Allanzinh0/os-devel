#pragma once

template <typename T> constexpr bool IsSigned(T number);

template <> constexpr bool IsSigned(char number) { return true; }
template <> constexpr bool IsSigned(short number) { return true; }
template <> constexpr bool IsSigned(int number) { return true; }
template <> constexpr bool IsSigned(long int number) { return true; }
template <> constexpr bool IsSigned(long long number) { return true; }

template <> constexpr bool IsSigned(unsigned char number) { return false; }
template <> constexpr bool IsSigned(unsigned short number) { return false; }
template <> constexpr bool IsSigned(unsigned int number) { return false; }
template <> constexpr bool IsSigned(unsigned long int number) { return false; }
template <> constexpr bool IsSigned(unsigned long long number) { return false; }
