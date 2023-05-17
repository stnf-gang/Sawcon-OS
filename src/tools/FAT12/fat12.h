// fat12.h
//
// main header file for the SCIM image manipulation tool
// This file was written for the SawconOS Host Tools
//
// Written: Sunday 14th May 2023
// Last Updated: Wednesday 17th May 2023
// 
// Written by Gabriel Jickells

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define SCIM_VERSION "0.0.02"

char *HelpPage[] = {
    "-h / --help : Displays information about how to use the tool",
    "-m / --mode : Sets the mode of the tool",
    "-d / --disk : Selects the disk image to manipulate",
    NULL
};

char *ModePage[] = {
    NULL,
    "read",
    NULL
};

enum ModeDefs {
    M_READ = 1,
};

typedef struct BootRecord_t {
    uint8_t DataSkip[3];                            // 3 bytes reserved for machine code instructions that skip past the data when the disk is booted
    // BIOS Parameter Block (BPB)
    char OEM_ID[8];                                 // contains the name of the tool or manufacturer that formatted the disk
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;                       // number of sectors reserved at the start of the disk for the boot sector and data and such
    uint8_t TotalFATs;                              // usually this value is 2 as to prevent data loss. Loads of old FAT drivers don't work right if its anything else
    uint16_t RootDirectoryEntries;                  // maximum number of files and other stuff that can fit in the root directory
    uint16_t TotalSectors;                          // if this is 0 then use the value in LargeSectors instead
    uint8_t DOS_MediaDesc;                          // used by early versions of MS-DOS to determine the type of disk
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;                       // used when the disk is booted to store drive geometry
    uint16_t TotalHeads;                            // used when the disk is booted to store drive geometry
    uint32_t HiddenSectors;                         // just the LBA of the start of the partition
    uint32_t LargeSectors;                          // used on disks larger than 32MiB
    // Extended BIOS Parameter Block (EBPB)
    uint8_t DriveNumber;                            // used when the disk is booted to store the disk in which the partition resides
    uint8_t _reserved;                              // reserved for use in Windows NT
    uint8_t Signature;                              // must be either 0x28 or 0x29 to indicate a valid file system
    uint32_t SerialID;
    char VolumeLabel[11];                           // name of the disk
    char SystemID[8];                               // specifies the version of FAT being used. Old disk formatting tools were very inconsistent with each other so this value probably won't be right
} __attribute__((packed)) BootRecord_t;             // tells gcc not to pad the struct. Makes reading the boot sector easier and more efficient

typedef struct DirectoryEntry_t {
    char Name[11];                                  // name of the file in "NAME    EXT" padded with spaces
    uint8_t Attributes;                             // contains information about the type of entry
    uint8_t _reserved;                              // reserved for use in Windows NT
    uint8_t CreationTimeCents;                      // CreationTime can only hold multiples of 2 as the seconds value so this offset is added on. Measured in hundredths of a second from 0 to 199
    uint16_t CreationTime;                          // CreationTime = (Hour << 11) | (Minute << 5) | (Second / 2)
    uint16_t CreationDate;                          // CreationDate = (Year << 9) | (Month << 5) | (Day)
    uint16_t AccessedDate;                          // same format as CreationDate. Contains the date of the last time the file was opened
    uint16_t FirstClusterHigh;                      // high 16 bits of the total 32 bit cluster number. This is never used in FAT12/16
    uint16_t ModificationTime;                      // same format as CreationTime. Contains the time of the last change to the file
    uint16_t ModificationDate;                      // same format as CreationDate. Contains the date of the last change to the file
    uint16_t FirstClusterLow;                       // low 16 bits of the total 32 bit cluster number. Used in Fat12/16/32
    uint32_t Size;                                  // measured in bytes
} __attribute__((packed)) DirectoryEntry_t;

BootRecord_t *ReadBootRecord(FILE* Disk);
bool ReadSectors(FILE *Disk, BootRecord_t *p_BootRecord, uint32_t LBA, uint32_t Count, void *BufferOut);
