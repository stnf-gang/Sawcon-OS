// scim.cpp
// 
// main source file for SCIM: the Sawcon Image Manipulator
// This file was written as part of the SawconOS Host Tools
// compiled using g++
// 
// Written: Saturday 12th August 2023
// Last Updated: Wednesday 16th August 2023
// 
// Written by Gabriel Jickells

#include "scim.hpp"

int main(int argc, char **argv) {
    char *DiskImageName = NULL; bool DiskImageNameSpecified = false;
    char *AffectedFileEntryName = NULL;

    // parse any arguments that were passed to the tool
    for(int i = 1; i < argc; i++) {
        if(!strcmp("-f", argv[i])) {
            // make sure this isn't the last argument
            if(i + 1 == argc) {
                std::cerr << "SCIM: Error - invalid argument usage\n";
                return -6;
            }
            AffectedFileEntryName = argv[++i];
        }
        else {
            if(!DiskImageNameSpecified) {
                DiskImageNameSpecified = true;
                DiskImageName = argv[i];
            }
            else {
                std::cerr << "SCIM: Error - Disk image name was specified too many times\n";
                return -1;
            }
        }
    }

    // make sure that a disk image name was specified
    if(!DiskImageName) {
        std::cerr << "SCIM: Error - Disk image was not specified\n";
        return -4;
    }

    // open the disk image in read mode so FAT information can be read from it
    FILE *DiskImageReadStream = fopen(DiskImageName, "rb");
    if(!DiskImageReadStream) {
        std::cerr << "SCIM: Error - Could not open disk image \"" << DiskImageName << "\"\n";
        return -2;
    }

    scim::FAT12::Disk FAT_Data;
    if(!FAT_Data.ReadBootRecord(DiskImageReadStream)) {
        std::cerr << "SCIM: Error - Could not read boot sector of disk image\n";
        return -3;
    }

    if(!FAT_Data.ReadRootDirectory(DiskImageReadStream)) {
        std::cerr << "SCIM: Error - Could not read root directory of disk image\n";
        return -5;
    }

    scim::FAT::DirectoryEntry_t *AffectedFileEntryData = FAT_Data.FindFile(AffectedFileEntryName);
    if(!AffectedFileEntryData) {
        std::cerr << "SCIM: Error - Could not find the file on the disk image\n";
        return -7;
    }

    if(!FAT_Data.ReadFAT(DiskImageReadStream)) {
        std::cerr << "SCIM: Error - Could not read the file allocation of the disk image\n";
        return -8;
    }

    scim::FAT::byte *FileBuffer = (scim::FAT::byte *)malloc((AffectedFileEntryData->Size + 1024));
    if(!FAT_Data.ReadFile(DiskImageReadStream, AffectedFileEntryData, FileBuffer)) {
        std::cerr << "SCIM: Error - Could not read the file from the disk\n";
        return -9;
    }

    // TESTING

    printf("%s\n", FileBuffer);

    // END OF TESTING

    return 0;
}
