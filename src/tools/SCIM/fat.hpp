// fat.hpp
//
// utilites and functions for FAT formatted disks
// This file was written as part of the Sawcon Image Manipulator
//
// Written: Sunday 13th August 2023
// Last Updated: Wednesday 16th August 2023
//
// Written by Gabriel Jickells

#pragma once

// if you are using this header as a standalone file for your own projects, replace this include with the following headers or include them in your projects main header file
// #include <stdio.h>
// #include <string.h>
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
            FAT::DirectoryEntry_t *RootDirectory = NULL;
            FAT::byte *FileAllocationTable = NULL;

            /// @brief read the FAT header data from the boot sector of a disk image into FS_Info
            /// @param Image disk image to read from
            /// @return true on success, false on failure
            bool ReadBootRecord(FILE *Image) {

                // navigate to where the FAT header is expected to be
                if(fseek(Image, FAT::BPB_OFFSET, SEEK_SET) < 0) 
                    return false;
                
                // because the BootRecord_t structure doesn't have any padding, it is possible to read the entire structures data all at once and all the values will still be in the right place
                if(fread(&FS_Info, sizeof(BootRecord_t), 1, Image) <= 0) 
                    return false;

                return true;
            }

            /// @brief read the file meta-data in the root directory of a disk image into RootDirectory. Requires FS_Info to have valid values in it
            /// @param Image disk image to read from
            /// @return true on success, false on failure
            bool ReadRootDirectory(FILE *Image) {
                
                // calculate the LBA of the root directory
                // the root directory is located directly after the FAT region of the disk, which is located after the reserved sectors at the start of the disk
                FAT::word RootDirectoryLBA = FS_Info.BPB.ReservedSectors + (FS_Info.BPB.TotalFATs * FS_Info.BPB.SectorsPerFAT);

                // calculate the size of the root directory in sectors
                unsigned int RootDirectoryBytes = sizeof(FAT::DirectoryEntry_t) * FS_Info.BPB.RootDirectoryEntries;
                // division operations always round down with integers so when a division needs to be rounded up, which it does in this case to make sure we read the whole thing, we can add the divisor - 1 to the dividend
                FAT::word RootDirectorySectors = (RootDirectoryBytes + (FS_Info.BPB.BytesPerSector - 1)) / FS_Info.BPB.BytesPerSector;

                // make space to store the root directory data when it is read
                // we shouldn't use RootDirectoryBytes here because if RootDirectorySectors is rounded to the next sector then RootDirectoryBytes will be inaccurate
                RootDirectory = (FAT::DirectoryEntry_t *)malloc(RootDirectorySectors * FS_Info.BPB.BytesPerSector);
                if(!RootDirectory) return false;

                // read the data
                if(!ReadSectors(Image, RootDirectoryLBA, RootDirectorySectors, RootDirectory))
                    return false;

                DataSectionLBA = RootDirectoryLBA + RootDirectorySectors;

                return true;

            }

            /// @brief Finds the meta data for a file entry in the root directory. Requires the root directory to have been read
            /// @param Name name of the file entry in "NAME    EXT" format
            /// @return pointer to the file entry data on success, NULL on failure
            FAT::DirectoryEntry_t *FindFile(const char *Name) {
                if(!RootDirectory) return NULL;
                for(int i = 0; i < FS_Info.BPB.RootDirectoryEntries; i++)
                    if(!memcmp(Name, RootDirectory[i].Name, 11))
                        return &RootDirectory[i];
                return NULL;
            }
 
            /// @brief Reads the entirety of a file on a disk image. Requires FS_Info, RootDirectory, and FileAllocationTable to all have valid values in them
            /// @param FileEntry File meta-data to get the files location on the disk
            /// @param BufferOut Buffer to store the data in, should be malloced before calling the function
            /// @return true on success, false on failure
            bool ReadFile(FILE *Image, FAT::DirectoryEntry_t *FileEntry, void *BufferOut) {
                FAT::byte *ByteBufferOut = (FAT::byte *)BufferOut;
                if(!FileAllocationTable) return false;
                FAT::word CurrentCluster = FileEntry->FirstClusterLow & 0xFFF;
                FAT::word CurrentLBA; FAT::word FAT_Index;
                while(CurrentCluster < LAST_CLUSTER) {
                    if(CurrentCluster == BAD_CLUSTER || CurrentCluster < FIRST_AVAILABLE_CLUSTER) return false;
                    CurrentLBA = Cluster2LBA(CurrentCluster);
                    if(!CurrentLBA) return false;
                    if(!ReadSectors(Image, CurrentLBA, FS_Info.BPB.SectorsPerCluster, ByteBufferOut))
                        return false;
                    ByteBufferOut += FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerCluster;
                    
                    // calculate the next cluster in the chain
                    FAT_Index = CurrentCluster * 3 / 2; // CurrentCluster is used as the index for the next cluster number, but CurrentCluster is a byte index but needs to be converted into a uint12 index
                    // computers don't natively support uint12 arrays so some bit fiddling is required to get the right value
                    if(CurrentCluster & 1) CurrentCluster = *(uint16_t *)(FileAllocationTable + FAT_Index) >> 4;
                    else CurrentCluster = *(uint16_t *)(FileAllocationTable + FAT_Index) & 0xFFF;
                }
                return true;
            }

            /// @brief Reads the first file allocation table of a disk image into FileAllocationTable
            /// @param Image disk image to read from
            /// @return true on sucess, false on failure
            bool ReadFAT(FILE *Image) {
                FileAllocationTable = (FAT::byte *)malloc(FS_Info.BPB.SectorsPerFAT * FS_Info.BPB.BytesPerSector);
                if(!FileAllocationTable) return false;
                if(!ReadSectors(Image, FS_Info.BPB.ReservedSectors, FS_Info.BPB.SectorsPerFAT, FileAllocationTable)) return false;
                return true;
            }

        private:

            FAT::word DataSectionLBA = 0;

            /// @brief read some sectors from a disk image. requires FS_Info to have valid values in it
            /// @param Image disk image to read from
            /// @param LBA which sector to start reading from
            /// @param Count how many sectors to read
            /// @param BufferOut where the data will be stored
            /// @return true on success, false on error
            bool ReadSectors(FILE *Image, FAT::word LBA, FAT::word Count, void *BufferOut) {
                if(fseek(Image, LBA * FS_Info.BPB.BytesPerSector, SEEK_SET) < 0) return false;
                if(fread(BufferOut, FS_Info.BPB.BytesPerSector, Count, Image) != Count) return false;
                return true;
            }

            /// @brief convert a cluster number to its LBA location. Requires DataSectionLBA to have been set by ReadRootDirectory
            /// @param Cluster FAT cluster number
            /// @return LBA number on success, 0 on failure
            FAT::word Cluster2LBA(FAT::word Cluster) {
                if(Cluster < FIRST_AVAILABLE_CLUSTER || 
                   !DataSectionLBA) return 0;
                return DataSectionLBA + (Cluster - FIRST_AVAILABLE_CLUSTER) * FS_Info.BPB.SectorsPerCluster;
            }

    };

}                           // contains FAT and cluster information exclusive to FAT12
