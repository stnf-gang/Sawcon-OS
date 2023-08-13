// fat.hpp
//
// utilites and functions for FAT formatted disks
// This file was written as part of the Sawcon Image Manipulator
//
// Written: Sunday 13th August 2023
// Last Updated: Sunday 13th August 2023
//
// Written by Gabriel Jickells

#pragma once
#include "scim.hpp"

namespace FAT {

    using byte = unsigned char;
    using word = unsigned short;
    using dword = unsigned int;

    typedef struct BiosParameterBlock_t {
        byte DataSkip[3];                               // instructions used in the boot sector code to skip past the FAT header to the actual code
        char OEM_Identifier[8];                         // name of the manufacturer that formatted the disk
        word BytesPerSector;
        byte SectorsPerCluster;                         // how big each chunk of a file is on a disk
        word ReservedSectors;                           // essentially the LBA of the FAT. This is the number of sectors reserved for boot code or other things
        byte TotalFATs;                                 // number of identical FATs for redundancy
        word RootDirectoryEntries;                      // maximum number of file entries that can fit in the root directory
        word TotalSectors;                              // size of the disk/disk partition in sectors
        byte MediaDescriptor;                           // used by old DOS versions to determine the type of disk
        word SectorsPerFAT;
        word SectorsPerTrack;                           // used in the boot sector code as a buffer for the disk geometry
        word TotalHeads;                                // used in the boot sector code as a buffer for the disk geometry
        dword HiddenSectors;                            // number of sectors before the start of the partition. Effectively the LBA of the partition
        dword LargeSectors;                             // used instead of TotalSectors if the disk has more than 65535 sectors (32MiB)
    } __attribute__((packed)) BiosParameterBlock_t;     // __attribute__((packed)) tells the compiler not to add any padding bytes. Usually this is done so null terminated arrays like strings end where they're supposed to. This is not the case in the BPB where every byte counts
    
    enum offsets {
        BPB_OFFSET = 0,
        EBPB_OFFSET = 36,
    } offsets;

    typedef struct DirectoryEntry_t {
        char Name[11];                                  // stored in "NAME    EXT" format
        byte Attributes;
        byte _reserved;                                 // reserved for use in windows NT
        byte CreationTimeCents;                         // adds hundreths of a second to the creation time. should only be able to go up to 199. This adds precision to the creation time which can only count seconds in multiples of 2
        word CreationTime;                              // CreationTime = (Hour << 11) | (minute << 5) | (Seconds >> 1)
        word CreationDate;                              // CreationDate = (year << 9) | (Month << 5) | (Day)
        word AccessedDate;                              // date of the last time that the file was opened
        word FirstClusterHigh;                          // upper 16 bits of the first cluster number. Not used in FAT12 and 16
        word ModificationTime;                          // time of the last write operation to the file
        word ModificationDate;                          // date of the last write operation to the file
        word FirstClusterLow;                           // lower 16 bits of the first cluster number
        dword Size;                                     // measured in bytes
    } __attribute__((packed)) DirectoryEntry_t;

}                           // contains FAT information standard for FAT12/16/32. Shouldn't be used for exFAT or vFAT

namespace FAT12 {
    
    enum ClusterInfo {
        FIRST_AVAILABLE_CLUSTER = 0x02,                 // cluster numbers start at 2
        BAD_CLUSTER = 0xff7,
        LAST_CLUSTER = 0xff8,
    } ClusterInfo;          // some cluster numbers are reserved for different purposes than indexing data on a disk
    
    typedef struct ExtendedBiosParameterBlock_t {
        FAT::byte DriveNumber;
        FAT::byte _reserved;                            // used for windows NT flags. Not that important for a generic driver but should probably be kept in mind
        FAT::byte Signature;                            // used to specify a valid file system. Must be either 0x28 or 0x29
        FAT::byte SerialID[4];                          // serial number of the drive. Sometimes set to date and time of the formatting but usually just zeroed out
        char VolumeLabel[11];                           // shows up as the disk name in file managers
        char SystemID[8];                               // usually contains "FAT12   " but this isn't standard so shouldn't be used for anything driver related
    } __attribute__((packed)) ExtendedBiosParameterBlock_t; // __attribute__((packed)) tells the compiler not to add any padding bytes. Usually this is done so null terminated arrays like strings end where they're supposed to. This is not the case in the BPB where every byte counts

    typedef struct BootRecord_t {
        FAT::BiosParameterBlock_t BPB;
        ExtendedBiosParameterBlock_t EBPB;
    } __attribute__((packed)) BootRecord_t;

    class Disk {
        
        public:

            BootRecord_t FS_Info;
            FAT::DirectoryEntry_t *RootDirectory;

            // read the disk image information from its boot record into Data
            bool ReadBootRecord(FILE *Image) {
                if(fseek(Image, FAT::BPB_OFFSET, SEEK_SET) == -1) return false;
                if(fread(&FS_Info.BPB, sizeof(FAT::BiosParameterBlock_t), 1, Image) <= 0) return false;
                if(fseek(Image, FAT::EBPB_OFFSET, SEEK_SET) == -1) return false;
                if(fread(&FS_Info.EBPB, sizeof(ExtendedBiosParameterBlock_t), 1, Image) <= 0) return false;
                return true;
            }

    };

}                           // contains FAT and cluster information exclusive to FAT12
