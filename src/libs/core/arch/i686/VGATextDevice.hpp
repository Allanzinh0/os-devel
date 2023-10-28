#pragma once

#include "../../dev/CharacterDevice.hpp"

#include <stddef.h>
#include <stdint.h>

namespace arch {
namespace i686 {

class VGATextDevice : public CharacterDevice {
public:
  VGATextDevice();
  virtual size_t Read(uint8_t *dataOut, size_t size) override;
  virtual size_t Write(const uint8_t *data, size_t size) override;

  void Clear();

  void PutChar(char chr, int x, int y);
  void PutColor(uint8_t color, int x, int y);

  void SetColor(uint8_t color) { m_Color = color; }
  uint8_t GetColor() const { return m_Color; }

private:
  char GetChar(int x, int y);
  uint8_t GetColor(int x, int y);

  void SetCursor(int x, int y);
  void Scrollback(int lines);

  void PutChar(char chr);

private:
  uint32_t m_ScreenX, m_ScreenY;
  uint8_t m_Color;
  static uint8_t *s_Buffer;
};

} // namespace i686
} // namespace arch
