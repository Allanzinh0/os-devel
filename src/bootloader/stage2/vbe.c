#include "vbe.h"

#include "memory.h"
#include "x86.h"

bool VBE_GetControllerInfo(VBE_InfoBlock *info) {
  if (x86_Video_GetVbeInfo(info) == 0x004f) {
    info->VideoModePtr = SEGOFF2LIN(info->VideoModePtr);

    return true;
  }

  return false;
}

bool VBE_GetModeInfo(uint16_t mode, VBE_ModeInfo *info) {
  return x86_Video_GetModeInfo(mode, info) == 0x004f;
}

bool VBE_SetMode(uint16_t mode) { return x86_Video_SetMode(mode) == 0x004f; }
