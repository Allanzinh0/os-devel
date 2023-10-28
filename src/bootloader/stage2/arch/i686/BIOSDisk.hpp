#pragma once

#include <core/dev/BlockDevice.hpp>

#include <stddef.h>
#include <stdint.h>

class BIOSDisk : public BlockDevice {
public:
  BIOSDisk(uint8_t device);
  virtual size_t Read(uint8_t *dataOut, size_t size) override;
  virtual size_t Write(const uint8_t *data, size_t size) override;
  virtual bool Seek(SeekPos pos, int rel) override;
  virtual size_t Size() override;
  virtual size_t Position() override;

  bool Initialize();

private:
  bool ReadNextSector();
  void LBA2CHS(uint64_t lba, uint16_t *cylinders, uint16_t *sectors,
               uint16_t *heads);

  static const constexpr int SectorSize = 512;

private:
  uint8_t m_ID;
  bool m_HaveExtensions;
  uint16_t m_Cylinders;
  uint16_t m_Sectors;
  uint16_t m_Heads;
  uint8_t m_Buffer[SectorSize];
  uint64_t m_Pos;
  uint64_t m_Size;
};
