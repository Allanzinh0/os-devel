#pragma once

#include <stdint.h>

constexpr int SectorSize = 512;

struct FATDirectoryEntry {
  uint8_t Name[11];
  uint8_t Attributes;
  uint8_t _Reserved;
  uint8_t CreatedTimeTenths;
  uint16_t CreatedTime;
  uint16_t CreatedDate;
  uint16_t AccessedDate;
  uint16_t FirstClusterHigh;
  uint16_t ModifiedTime;
  uint16_t ModifiedDate;
  uint16_t FirstClusterLow;
  uint32_t Size;
} __attribute__((packed));

struct FATLongFileEntry {
  uint8_t Order;
  int16_t Chars1[5];
  uint8_t Attributes;
  uint8_t LongEntryType;
  uint8_t Checksum;
  int16_t Chars2[6];
  uint16_t _AlwaysZero;
  int16_t Chars3[6];
} __attribute__((packed));

struct FATExtendedBootRecord {
  uint8_t DriveNumber;
  uint8_t _Reserved;
  uint8_t Signature;
  uint32_t VolumeId;
  uint8_t VolumeLabel[11];
  uint8_t SystemId[8];
} __attribute__((packed));

struct FAT32ExtendedBootRecord {
  uint32_t SectorsPerFat;
  uint16_t Flags;
  uint16_t FATVersionNumber;
  uint32_t RootDirCluster;
  uint16_t FSInfoSector;
  uint16_t BackupBootSector;
  uint8_t _Reserved[12];
  FATExtendedBootRecord EBR;
} __attribute__((packed));

struct FATBootSector {
  uint8_t BootJumpInstruction[3];
  uint8_t OemIdentifier[8];
  uint16_t BytesPerSector;
  uint8_t SectorsPerCluster;
  uint16_t ReservedSectors;
  uint8_t FatCount;
  uint16_t DirEntriesCount;
  uint16_t TotalSectors;
  uint8_t MediaDescriptorType;
  uint16_t SectorsPerFat;
  uint16_t SectorsPerTrack;
  uint16_t Heads;
  uint32_t HiddenSectors;
  uint32_t LargeSectorCount;

  // extended boot record
  union {
    FATExtendedBootRecord EBR1216;
    FAT32ExtendedBootRecord EBR32;
  };
} __attribute__((packed));

enum FAT_Attributes {
  FAT_ATTRIBUTE_READ_ONLY = 0x01,
  FAT_ATTRIBUTE_HIDDEN = 0x02,
  FAT_ATTRIBUTE_SYSTEM = 0x04,
  FAT_ATTRIBUTE_VOLUME_ID = 0x08,
  FAT_ATTRIBUTE_DIRECTORY = 0x10,
  FAT_ATTRIBUTE_ARCHIVE = 0x20,
  FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN |
                      FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID |
                      FAT_ATTRIBUTE_DIRECTORY | FAT_ATTRIBUTE_ARCHIVE
};
