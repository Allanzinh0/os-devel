#include "RangeBlockDevice.hpp"

#include <core/Debug.hpp>
#include <core/cpp/Algorithm.hpp>

RangeBlockDevice::RangeBlockDevice() {}

RangeBlockDevice::RangeBlockDevice(BlockDevice *device, size_t rangeBegin,
                                   size_t rangeSize)
    : m_Device(device), m_RangeBegin(rangeBegin), m_RangeSize(rangeSize) {
  m_Device->Seek(SeekPos::Set, rangeBegin);
}

size_t RangeBlockDevice::Read(uint8_t *data, size_t size) {
  if (m_Device == nullptr)
    return 0;

  size = Min(size, Size() - Position());

  size_t ret = m_Device->Read(data, size);
  return ret;
}

size_t RangeBlockDevice::Write(const uint8_t *data, size_t size) {
  if (m_Device == nullptr)
    return 0;

  size = Min(size, Size() - Position());

  return m_Device->Write(data, size);
}

bool RangeBlockDevice::Seek(SeekPos pos, int rel) {
  if (m_Device == nullptr)
    return false;

  switch (pos) {
  case SeekPos::Set:
    return m_Device->Seek(pos, m_RangeBegin + rel);
    break;
  case SeekPos::Current:
    return m_Device->Seek(pos, rel);
    break;
  case SeekPos::End:
    return m_Device->Seek(pos, m_RangeBegin + m_RangeSize);
    break;
  default:
    return false;
    break;
  }
}

size_t RangeBlockDevice::Size() { return m_RangeSize; }

size_t RangeBlockDevice::Position() {
  return m_Device->Position() - m_RangeBegin;
}
