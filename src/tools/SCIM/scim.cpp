// scim.cpp
// 
// main source file for SCIM: the Sawcon Image Manipulator
// This file was written as part of the SawconOS Host Tools
// compiled using g++
// 
// Written: Saturday 12th August 2023
// Last Updated: Sunday 13th August 2023
// 
// Written by Gabriel Jickells

#include "scim.hpp"

int main(int argc, char **argv) {
    char *DiskImageName = NULL; bool DiskImageNameSpecified = false;
    
    // parse any arguments that were passed to the tool
    for(int i = 1; i < argc; i++) {
        if(!DiskImageNameSpecified) {
            DiskImageNameSpecified = true;
            DiskImageName = argv[i];
        }
        else {
            std::cerr << "SCIM: Error - Disk image name was specified too many times\n";
            return -1;
        }
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

    // TESTING

    printf("Disk Image OEM Identifier: \"%.8s\"\n", FAT_Data.FS_Info.BPB.OEM_Identifier);
    printf("Disk Signature: 0x%hhx\n", FAT_Data.FS_Info.EBPB.Signature);

    // END OF TESTING

    return 0;
}
