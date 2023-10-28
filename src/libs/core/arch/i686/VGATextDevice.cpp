#include "VGATextDevice.hpp"

#include "IO.hpp"

namespace arch {
namespace i686 {

static inline constexpr unsigned ScreenWidth = 80;
static inline constexpr unsigned ScreenHeight = 25;
static inline constexpr uint8_t DefaultColor = 0x7;

uint8_t *VGATextDevice::s_Buffer = (uint8_t *)0xB8000;

VGATextDevice::VGATextDevice()
    : m_ScreenX(0), m_ScreenY(0), m_Color(DefaultColor) {
  Clear();
}

size_t VGATextDevice::Read(uint8_t *dataOut, size_t size) { return 0; }

size_t VGATextDevice::Write(const uint8_t *data, size_t size) {
  for (int i = 0; i < size; i++) {
    PutChar((char)data[i]);
  }

  return size;
}

void VGATextDevice::Clear() {
  for (int y = 0; y < ScreenHeight; y++) {
    for (int x = 0; x < ScreenWidth; x++) {
      PutChar(' ', x, y);
      PutColor(DefaultColor, x, y);
    }
  }

  m_ScreenX = 0;
  m_ScreenY = 0;
  SetCursor(m_ScreenX, m_ScreenY);
}

void VGATextDevice::PutChar(char chr, int x, int y) {
  s_Buffer[2 * (y * ScreenWidth + x)] = chr;
}

void VGATextDevice::PutColor(uint8_t color, int x, int y) {
  s_Buffer[2 * (y * ScreenWidth + x) + 1] = color;
}

char VGATextDevice::GetChar(int x, int y) {
  return s_Buffer[2 * (y * ScreenWidth + x)];
}

uint8_t VGATextDevice::GetColor(int x, int y) {
  return s_Buffer[2 * (y * ScreenWidth + x) + 1];
}

void VGATextDevice::SetCursor(int x, int y) {
  int pos = y * ScreenWidth + x;

  OutB(0x3D4, 0x0F);
  OutB(0x3D5, (uint8_t)(pos & 0xFF));

  OutB(0x3D4, 0x0E);
  OutB(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void VGATextDevice::Scrollback(int lines) {
  for (int y = lines; y < ScreenHeight; y++)
    for (int x = 0; x < ScreenWidth; x++) {
      PutChar(GetChar(x, y), x, y - lines);
      PutColor(GetColor(x, y), x, y - lines);
    }

  for (int y = ScreenHeight - lines; y < ScreenHeight; y++)
    for (int x = 0; x < ScreenWidth; x++) {
      PutChar(x, y, ' ');
      PutColor(x, y, DefaultColor);
    }

  m_ScreenY -= lines;
  SetCursor(m_ScreenX, m_ScreenY);
}

void VGATextDevice::PutChar(char chr) {
  switch (chr) {
  case '\n':
    m_ScreenX = 0;
    m_ScreenY++;
    break;
  case '\r':
    m_ScreenX = 0;
    break;
  case '\t':
    for (int i = 0; i < 4 - (m_ScreenX % 4); i++)
      PutChar(' ');
  default:
    PutChar(chr, m_ScreenX, m_ScreenY);
    PutColor(DefaultColor, m_ScreenX, m_ScreenY);
    m_ScreenX++;
    break;
  }

  if (m_ScreenX >= ScreenWidth) {
    m_ScreenX = 0;
    m_ScreenY++;
  }

  if (m_ScreenY >= ScreenHeight)
    Scrollback(1);

  SetCursor(m_ScreenX, m_ScreenY);
}

} // namespace i686
} // namespace arch
