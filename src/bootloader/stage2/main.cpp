#include "arch/i686/BIOSDisk.hpp"
#include "arch/i686/RealMemory.hpp"
#include "cpp/NewDelete.hpp"
#include "dev/MBR.hpp"
#include "fs/ELFFile.hpp"
#include "memdefs.h"
#include "memory/Stage2Allocator.hpp"

#include <core/Debug.hpp>
#include <core/Defs.hpp>
#include <core/arch/i686/E9Device.hpp>
#include <core/arch/i686/VGATextDevice.hpp>
#include <core/dev/BlockDevice.hpp>
#include <core/dev/RangeBlockDevice.hpp>
#include <core/dev/TextDevice.hpp>
#include <core/fs/FATFileSystem.hpp>

#include <stddef.h>
#include <stdint.h>

using KernelStart = void();

void Stage2Start(uint16_t bootDrive, void *partition) {
  Debug::Info("Stage2", "Enter 32-bit protected mode");

  BIOSDisk disk(bootDrive);
  if (!disk.Initialize()) {
    Debug::Critical("Stage2", "Disk init error!");
    return;
  }

  // Handle partitioned disk
  BlockDevice *part;
  RangeBlockDevice partRange;

  if (bootDrive < 0x80) {
    part = &disk;
  } else {
    MBREntry *entry = ToLinear<MBREntry *>((uint32_t)partition);

    partRange = RangeBlockDevice(&disk, entry->LBAStart * SectorSize,
                                 entry->Size * SectorSize);
    part = &partRange;
  }

  // Read partition
  FATFileSystem filesystem;
  if (!filesystem.Initialize(part)) {
    Debug::Critical("Stage2", "FAT filesystem init error!");
    return;
  }

  Debug::Info("Stage2", "Loading kernel file.");
  File *kernel = filesystem.Open("/BOOT/KERNEL.ELF", FileOpenMode::Read);
  if (kernel == nullptr) {
    Debug::Error("Stage2", "Kernel file not found!");
    return;
  }

  Debug::Info("Stage2", "Reading elf program.");
  ELFFile kernelProgram;
  if (!kernelProgram.Load(kernel)) {
    Debug::Error("Stage2", "Kernel elf format could not be readed!");
    return;
  }

  Debug::Info("Stage2", "Running kernel.elf");
  KernelStart *kernelEntry = (KernelStart *)kernelProgram.GetEntrypoint();
  Debug::Debug("Stage2", "Kernel entrypoint: 0x%x", kernelEntry);
  kernelEntry();
}

EXPORT void ASMCALL Start(uint16_t bootDrive, void *partition) {
  Stage2Allocator g_Allocator((void *)MEMORY_MIN, MEMORY_MAX - MEMORY_MIN);
  SetCppAllocator(&g_Allocator);

  arch::i686::VGATextDevice g_VGADevice;
  arch::i686::E9Device g_DebugDevice;

  g_VGADevice.Clear();
  TextDevice screen(&g_VGADevice);
  Debug::AddOutputDevice(Debug::Level::Info, &screen, false);

  TextDevice debugScreen(&g_DebugDevice);
  Debug::AddOutputDevice(Debug::Level::Debug, &debugScreen, true);

  Stage2Start(bootDrive, partition);

  Debug::Warn("Stage2", "Halt processor!");
  for (;;)
    ;
}
