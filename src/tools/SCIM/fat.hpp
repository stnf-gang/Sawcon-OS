// fat.hpp
//
// utilites and functions for FAT formatted disks
// This file was written as part of the Sawcon Image Manipulator
// This version of the header was written for SCIM Alpha 1.3
//
// Written: Sunday 13th August 2023
// Last Updated: Saturday 7th October 2023
//
// Written by Gabriel Jickells

#pragma once

// if you are using this header as a standalone file for your 
// own projects, replace #include "scim.hpp" with the 
// following code:
/*
#include <stdio.h>
#include <string.h>
#includr <ctime>

namespace scim {
    using byte = unsigned char;
    using word = unsigned short;
    using dword = unsigned int;
}
*/ 
#include "scim.hpp"

namespace FAT {

    typedef struct BiosParameterBlock_t {
        scim::byte DataSkip[3];                         // instructions used in the boot sector code to skip past the FAT header to the actual code
        char OEM_Identifier[8];                         // name of the manufacturer that formatted the disk
        scim::word BytesPerSector;
        scim::byte SectorsPerCluster;                   // how big each chunk of a file is on a disk
        scim::word ReservedSectors;                     // essentially the LBA of the FAT. This is the number of sectors reserved for boot code or other things
        scim::byte TotalFATs;                           // number of identical FATs for redundancy
        scim::word RootDirectoryEntries;                // maximum number of file entries that can fit in the root directory
        scim::word TotalSectors;                        // size of the disk/disk partition in sectors
        scim::byte MediaDescriptor;                     // used by old DOS versions to determine the type of disk
        scim::word SectorsPerFAT;
        scim::word SectorsPerTrack;                     // used in the boot sector code as a buffer for the disk geometry
        scim::word TotalHeads;                          // used in the boot sector code as a buffer for the disk geometry
        scim::dword HiddenSectors;                      // number of sectors before the start of the partition. Effectively the LBA of the partition
        scim::dword LargeSectors;                       // used instead of TotalSectors if the disk has more than 65535 sectors (32MiB)
    } __attribute__((packed)) BiosParameterBlock_t;     // __attribute__((packed)) tells the compiler not to add any padding bytes. Usually this is done so null terminated arrays like strings end where they're supposed to. This is not the case in the BPB where every byte counts
    
    enum offsets {
        BPB_OFFSET = 0,
        EBPB_OFFSET = 36,
    } offsets;

    typedef struct DirectoryEntry_t {
        char Name[11];                                  // stored in "NAME    EXT" format
        scim::byte Attributes;
        scim::byte _reserved;                           // reserved for use in windows NT
        scim::byte CreationTimeCents;                   // adds hundreths of a second to the creation time. should only be able to go up to 199. This adds precision to the creation time which can only count seconds in multiples of 2
        scim::word CreationDate;                        // CreationDate = ((year - 1980) << 9) | (Month << 5) | (Day)
        scim::word AccessedDate;                        // date of the last time that the file was opened
        scim::word CreationTime;                        // CreationTime = (Hour << 11) | (minute << 5) | (Seconds >> 1)
        scim::word FirstClusterHigh;                    // upper 16 bits of the first cluster number. Not used in FAT12 and 16
        scim::word ModificationTime;                    // time of the last write operation to the file
        scim::word ModificationDate;                    // date of the last write operation to the file
        scim::word FirstClusterLow;                     // lower 16 bits of the first cluster number
        scim::dword Size;                               // measured in bytes
    } __attribute__((packed)) DirectoryEntry_t;

    // the first character of a directory entry's name can have some special functions
    enum NAME_SPECIAL {
        I_SPECIALCHAR = (char)0,                        // index into the name where the special character is located
        N_END = (char)0x00,                             // directories are NULL terminated
        N_SIGMALOW = (char)0x05,                        // ASCII code 0xe5 is a FAT special character so 0x05 is used to specify lowercase sigma which is the symbol for ASCII code 0xe5
        N_ENTRYFREE = (char)0xe5,                       // usually means there was a file entry here but it was deleted
    };

    enum FILE_ATTRIBUTES {
        A_READ_ONLY =   0b00000001,                     // means the entry cannot be written to
        A_HIDDEN =      0b00000010,                     // means the entry exists but is not meant to be visible
        A_SYSTEM =      0b00000100,                     // means the entry is used by an operating system to function
        A_VOLUME_NAME = 0b00001000,                     // means the entry specifies the name of the disk
        A_DIRECTORY =   0b00010000,                     // means the entry is a sub-directory
        A_ARCHIVE =     0b00100000,                     // means the entry is a file
    };

    scim::word localtime2FatTime(tm *LocalTime) {
        return (LocalTime->tm_hour << 11) | (LocalTime->tm_min << 5) | (LocalTime->tm_sec >> 1);
    }

    scim::word localtime2FatDate(tm *LocalTime) {
        // tm_year starts at 1900 but FAT year starts at 1980
        // tm_mon starts at 0 but FAT month starts at 1
        return ((LocalTime->tm_year - 80) << 9) | ((LocalTime->tm_mon + 1) << 5) | (LocalTime->tm_mday);
    }

}                           // contains FAT information standard for FAT12/16/32. Shouldn't be used for exFAT or vFAT

namespace FAT12 {
    
    enum ClusterInfo {
        FIRST_AVAILABLE_CLUSTER = 0x02,                 // cluster numbers start at 2
        BAD_CLUSTER = 0xff7,
        LAST_CLUSTER = 0xff8,
    } ClusterInfo;          // some cluster numbers are reserved for different purposes than indexing data on a disk
    
    typedef struct ExtendedBiosParameterBlock_t {
        scim::byte DriveNumber;
        scim::byte _reserved;                           // used for windows NT flags. Not that important for a generic driver but should probably be kept in mind
        scim::byte Signature;                           // used to specify a valid file system. Must be either 0x28 or 0x29
        scim::dword SerialID;                           // serial number of the drive. Sometimes set to date and time of the formatting but usually just zeroed out
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
            scim::byte *FileAllocationTable = NULL;

            /// @brief Sets the FS_Info, RootDirectory, and FileAllocationTable variables to 
            /// values suited for handling the disk image
            /// @param Image disk image to set the values for
            /// @return true on success, false on failure
            bool Initialise(FILE *Image) {

                // read the bootrecord and store its values in FS_Info
                if(!ReadBootRecord(Image)) return false;

                // read the root directory and store a pointer to its data in RootDirectory
                if(!ReadRootDirectory(Image)) return false;

                // read the FAT and store a pointer to its data in FileAllocationTable
                if(!ReadFAT(Image)) return false;

                return true;

            }

            /// @brief free any memory that was allocated by Disk functions
            void Clean() {
                free(RootDirectory);
                free(FileAllocationTable);
            }

            /// @brief Finds the meta data for a file entry in the root directory. Requires the root directory to have been read
            /// @param Name name of the file entry in "NAME    EXT" format
            /// @return pointer to the file entry data on success, NULL on failure
            FAT::DirectoryEntry_t *FindEntry(const char *Name) {
                if(!RootDirectory) return NULL;
                for(int i = 0; i < FS_Info.BPB.RootDirectoryEntries; i++)
                    if(!memcmp(Name, RootDirectory[i].Name, 11))
                        return &RootDirectory[i];
                return NULL;
            }
 
            /// @brief Reads the entirety of a file on a disk image. Requires FS_Info, RootDirectory, and FileAllocationTable to all have valid values in them
            /// @param Image disk image to read from
            /// @param FileEntry File meta-data to get the files location on the disk
            /// @param BufferOut Buffer to store the data in, should be malloced before calling the function
            /// @return true on success, false on failure
            bool ReadEntry(FILE *Image, FAT::DirectoryEntry_t *FileEntry, void *BufferOut) {
                scim::byte *ByteBufferOut = (scim::byte *)BufferOut;
                if(!FileAllocationTable) return false;
                scim::word CurrentCluster = FileEntry->FirstClusterLow & 0xFFF;
                scim::word CurrentLBA; scim::word FAT_Index;
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
                    if(CurrentCluster & 1) CurrentCluster = *(scim::word *)(FileAllocationTable + FAT_Index) >> 4;
                    else CurrentCluster = *(scim::word *)(FileAllocationTable + FAT_Index) & 0xFFF;
                }
                return true;
            }

            /// @brief Deletes a file entry on the disk
            /// @param Image disk image to rid of the file
            /// @return true on success, false on failure
            bool DeleteEntry(FILE *Image, FAT::DirectoryEntry_t *FileEntry) {
                scim::word CurrentCluster = FileEntry->FirstClusterLow & 0xFFF;
                scim::word FAT_Index;
                do {
                    if(CurrentCluster < FIRST_AVAILABLE_CLUSTER) return false;
                    // same kind of uint12_t messing from ReadEntry
                    FAT_Index = CurrentCluster * 3 / 2;
                    if(CurrentCluster & 1) {
                        CurrentCluster = *(scim::word *)(FileAllocationTable + FAT_Index) >> 4;
                        *(scim::word *)(FileAllocationTable + FAT_Index) &= 0x000F;         // the upper 12 bits are used for the cluster number here, leave the low 4 untouched
                    }
                    else {
                        CurrentCluster = *(scim::word *)(FileAllocationTable + FAT_Index) & 0xFFF;
                        *(scim::word *)(FileAllocationTable + FAT_Index) &= 0xF000;         // the lower 12 bits are used for the cluster number here, leave the top 4 untouched
                    }
                } while(CurrentCluster < LAST_CLUSTER);

                // find the entry again to get a reference to its location in the root directory
                FAT::DirectoryEntry_t *EntryReference = FindEntry(FileEntry->Name);
                if(!EntryReference) return false;

                // set the special character in the name to mark it as deleted
                EntryReference->Name[FAT::I_SPECIALCHAR] = FAT::N_ENTRYFREE;
                EntryReference->FirstClusterLow = 0;

                // write the updated entry data and FAT to the disk image
                if(!WriteEntryData(Image, EntryReference, (EntryReference - RootDirectory) / sizeof(FAT::DirectoryEntry_t))) return false;
                if(!WriteFAT(Image)) return false;

                return true;
            }

            bool CreateEntry(FILE *Image, const char *Name, scim::byte Attributes, FILE *FileData) {
                if(strlen(Name) != 11) return false;
                for(int i = 0; i < FS_Info.BPB.RootDirectoryEntries; i++) {
                    // prevent duplicate entries
                    if(!memcmp(Name, RootDirectory[i].Name, 11)) return false;
                    
                    if(RootDirectory[i].Name[0] == FAT::N_END || RootDirectory[i].Name[0] == FAT::N_ENTRYFREE) {
                        // get the current time and date
                        time_t now = time(0);
                        tm *NowLocal = localtime(&now);

                        // fill the entry data with valid values
                        strcpy(RootDirectory[i].Name, Name);
                        RootDirectory[i].Attributes = Attributes;
                        RootDirectory[i]._reserved = 0;
                        if(NowLocal->tm_sec & 1) RootDirectory[i].CreationTimeCents = 100;
                        else RootDirectory[i].CreationTimeCents = 0;
                        RootDirectory[i].CreationDate = FAT::localtime2FatDate(NowLocal);
                        RootDirectory[i].AccessedDate = FAT::localtime2FatDate(NowLocal);
                        RootDirectory[i].CreationTime = FAT::localtime2FatTime(NowLocal);
                        RootDirectory[i].FirstClusterHigh = 0;
                        RootDirectory[i].ModificationTime = FAT::localtime2FatTime(NowLocal);
                        RootDirectory[i].ModificationDate = FAT::localtime2FatDate(NowLocal);
                        RootDirectory[i].FirstClusterLow = NextFreeCluster();
                        
                        // get the size of the file
                        if(fseek(FileData, 0, SEEK_END) < 0) return false;
                        RootDirectory[i].Size = ftell(FileData);
                        
                        if(!WriteEntryData(Image, &RootDirectory[i], i)) return false;

                        // calculate how many values need to be added to the FAT
                        size_t FileClusterCount = RootDirectory[i].Size / (FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerCluster);
                        if(RootDirectory[i].Size % (FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerCluster)) FileClusterCount++;

                        // fill the values in the file allocation table
                        scim::word FatIndex;
                        scim::word CurrentCluster = RootDirectory[i].FirstClusterLow;
                        for(size_t j = 0; j < FileClusterCount; j++) {
                            FatIndex = CurrentCluster * 3 / 2;
                            if(j + 1 == FileClusterCount) {
                                if(CurrentCluster & 1) *(scim::word *)(FileAllocationTable + FatIndex) |= LAST_CLUSTER << 4;
                                else *(scim::word *)(FileAllocationTable + FatIndex) |= LAST_CLUSTER;
                                break;
                            }
                            if(CurrentCluster & 1) {
                                // fill the active cluster with a dummy number first so it doesnt get counted as free
                                *(scim::word *)(FileAllocationTable + FatIndex) |= 0x001 << 4;
                                CurrentCluster = NextFreeCluster();
                                // empty the dummy cluster number
                                *(scim::word *)(FileAllocationTable + FatIndex) &= 0x000f;
                                *(scim::word *)(FileAllocationTable + FatIndex) |= CurrentCluster << 4;
                            }
                            else {
                                *(scim::word *)(FileAllocationTable + FatIndex) |= 0x001;
                                CurrentCluster = NextFreeCluster();
                                *(scim::word *)(FileAllocationTable + FatIndex) &= 0xf000;
                                *(scim::word *)(FileAllocationTable + FatIndex) |= CurrentCluster;
                            }
                        }

                        if(!WriteFAT(Image)) return false;

                        // now the location of the data is known on the disk, all that is left to do is fill all of that data
                        CurrentCluster = RootDirectory[i].FirstClusterLow;
                        scim::byte *FileDataBuffer = (scim::byte *)malloc(FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerCluster);
                        for(int j = 0; CurrentCluster < LAST_CLUSTER; j++) {
                            FatIndex = CurrentCluster * 3 / 2;

                            memset(FileDataBuffer, 0, FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerCluster);
                            if(fseek(FileData, j * FS_Info.BPB.SectorsPerCluster * FS_Info.BPB.BytesPerSector, SEEK_SET) < 0) return false;
                            if(!fread(FileDataBuffer, scim::min(FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerCluster, ftell(FileData) - RootDirectory[i].Size), 1, FileData)) {
                                if(ferror(FileData)) return false;
                            }
                            
                            if(fseek(Image, Cluster2LBA(CurrentCluster) * FS_Info.BPB.BytesPerSector, SEEK_SET) < 0) return false;
                            if(!fwrite(FileDataBuffer, FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerCluster, 1, Image)) return false;

                            if(CurrentCluster & 1) CurrentCluster = *(scim::word *)(FileAllocationTable + FatIndex) >> 4;
                            else CurrentCluster = *(scim::word *)(FileAllocationTable + FatIndex) & 0xFFF;
                        }

                        free(FileDataBuffer);
                        return true;
                    }
                }
                return false;
            }

            /// @brief Writes the data currently in FS_Info into the boot record of the disk. Other data in the boot record is unaffected
            /// @param Image the image to write to
            /// @return true for success, false for failure
            bool WriteBootRecord(FILE *Image) {
                if(fseek(Image, 0, SEEK_SET) < 0) return false;
                if(!fwrite(&FS_Info, sizeof(BootRecord_t), 1, Image)) return false; 
                return true;
            }

        private:

            scim::word DataSectionLBA = 0;

            /// @brief read some sectors from a disk image. requires FS_Info to have valid values in it
            /// @param Image disk image to read from
            /// @param LBA which sector to start reading from
            /// @param Count how many sectors to read
            /// @param BufferOut where the data will be stored
            /// @return true on success, false on error
            bool ReadSectors(FILE *Image, scim::word LBA, scim::word Count, void *BufferOut) {
                if(fseek(Image, LBA * FS_Info.BPB.BytesPerSector, SEEK_SET) < 0) return false;
                if(fread(BufferOut, FS_Info.BPB.BytesPerSector, Count, Image) != Count) return false;
                return true;
            }

            /// @brief convert a cluster number to its LBA location. Requires DataSectionLBA to have been set by ReadRootDirectory
            /// @param Cluster FAT cluster number
            /// @return LBA number on success, 0 on failure
            scim::word Cluster2LBA(scim::word Cluster) {
                if(Cluster < FIRST_AVAILABLE_CLUSTER || 
                   !DataSectionLBA) return 0;
                return DataSectionLBA + (Cluster - FIRST_AVAILABLE_CLUSTER) * FS_Info.BPB.SectorsPerCluster;
            }

            /// @brief Reads the first file allocation table of a disk image into FileAllocationTable
            /// @param Image disk image to read from
            /// @return true on sucess, false on failure
            bool ReadFAT(FILE *Image) {
                FileAllocationTable = (scim::byte *)malloc(FS_Info.BPB.SectorsPerFAT * FS_Info.BPB.BytesPerSector);
                if(!FileAllocationTable) return false;
                if(!ReadSectors(Image, FS_Info.BPB.ReservedSectors, FS_Info.BPB.SectorsPerFAT, FileAllocationTable)) return false;
                return true;
            }

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
                scim::word RootDirectoryLBA = FS_Info.BPB.ReservedSectors + (FS_Info.BPB.TotalFATs * FS_Info.BPB.SectorsPerFAT);

                // calculate the size of the root directory in sectors
                unsigned int RootDirectoryBytes = sizeof(FAT::DirectoryEntry_t) * FS_Info.BPB.RootDirectoryEntries;
                // division operations always round down with integers so when a division needs to be rounded up, which it does in this case to make sure we read the whole thing, we can add the divisor - 1 to the dividend
                scim::word RootDirectorySectors = (RootDirectoryBytes + (FS_Info.BPB.BytesPerSector - 1)) / FS_Info.BPB.BytesPerSector;

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

            bool WriteEntryData(FILE* Image, FAT::DirectoryEntry_t *Entry, scim::word index) {
                scim::word RootDirectoryLBA = FS_Info.BPB.ReservedSectors + (FS_Info.BPB.TotalFATs * FS_Info.BPB.SectorsPerFAT);
                
                if(fseek(Image, (RootDirectoryLBA * FS_Info.BPB.BytesPerSector) + (index * sizeof(FAT::DirectoryEntry_t)), SEEK_SET) < 0) return false;
                if(fwrite(Entry, sizeof(FAT::DirectoryEntry_t), 1, Image) <= 0) return false;
                
                return true;
            }

            bool WriteFAT(FILE *Image) {
                if(fseek(Image, FS_Info.BPB.ReservedSectors * FS_Info.BPB.BytesPerSector, SEEK_SET) < 0) return false;
                if(fwrite(FileAllocationTable, FS_Info.BPB.BytesPerSector * FS_Info.BPB.SectorsPerFAT, 1, Image) <= 0) return false;
                return true;
            }

            /// @brief finds the next cluster in the FAT that isnt being used
            /// @return the cluster number of the first free cluster
            scim::word NextFreeCluster() {
                scim::word FatIndex;
                for(int i = FIRST_AVAILABLE_CLUSTER;; i++) {
                    FatIndex = i * 3 / 2;
                    if((i & 1) && (*(scim::word *)(FileAllocationTable + FatIndex) >> 4) == 0) return i;
                    if(!(i & 1) && (*(scim::word *)(FileAllocationTable + FatIndex) & 0xfff) == 0) return i;
                }
            }

    };

}                           // contains FAT and cluster information exclusive to FAT12
