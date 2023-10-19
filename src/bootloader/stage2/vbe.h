#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  char VbeSignature[4];
  uint16_t VbeVersion;
  uint16_t OemStringPtr[2];
  uint8_t Capabilities[4];
  uint32_t VideoModePtr;
  uint16_t TotalMemory;
  uint8_t _Reserved[236 + 256];
} __attribute__((packed)) VBE_InfoBlock;

typedef struct {
  uint16_t Attributes;
  uint8_t WindowA;
  uint8_t WindowB;
  uint16_t Granularity;
  uint16_t WindowSize;
  uint16_t SegmentA;
  uint16_t SegmentB;
  uint32_t WinFuncPtr;
  uint16_t Pitch;
  uint16_t Width;
  uint16_t Height;
  uint8_t WChar;
  uint8_t YChar;
  uint8_t Planes;
  uint8_t BPP;
  uint8_t banks;
  uint8_t MemoryModel;
  uint8_t BankSize;
  uint8_t ImagePanes;
  uint8_t _Reserved0;

  uint8_t RedMask;
  uint8_t RedPosition;
  uint8_t GreenMask;
  uint8_t GreenPosition;
  uint8_t BlueMask;
  uint8_t BluePosition;
  uint8_t ReservedMask;
  uint8_t ReservedPosition;
  uint8_t DirectColorAttributes;

  uint32_t Framebuffer;
  uint32_t OffScreenMemOff;
  uint32_t OffScreenMemSize;
  uint8_t _Reserved1[206];
} __attribute__((packed)) VBE_ModeInfo;

bool VBE_GetControllerInfo(VBE_InfoBlock *info);
bool VBE_GetModeInfo(uint16_t mode, VBE_ModeInfo *info);
bool VBE_SetMode(uint16_t mode);
