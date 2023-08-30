// scim.cpp
// 
// main source file for SCIM: the Sawcon Image Manipulator
// This file was written as part of the SawconOS Host Tools
// This version of the code was written for SCIM Alpha 1.3
// compiled using g++
// 
// Written: Saturday 12th August 2023
// Last Updated: Wednesday 30th August 2023
// 
// Written by Gabriel Jickells

#include "scim.hpp"

int main(int argc, char **argv) {

    // - argv[scim::MODE_INDEX] is expected to specify the mode that SCIM will 
    // operate in so its pretty important that there is an argument there
    // - argv uses indexes starting at 0 whereas argc uses indexes starting at 1
    // so 1 must be taken away from argc to account for the offset
    if(argc - 1 < scim::MODE_INDEX) {
        std::cerr << "SCIM: Error - Not enough arguments\n";
        return -1;
    }

    // parse the mode argument

    unsigned int mode = scim::M_INVALID;

    // iterate through the list of valid modes until a valid one is found
    for(int i = 0; scim::ValidModes[i] != NULL; i++)
        if(!strcasecmp(argv[scim::MODE_INDEX], scim::ValidModes[i])) {
            mode = i + 1;               // index 0 is reserved for invalid modes so an offset needs to be added
            break;
        }

    if(mode == scim::M_INVALID) {
        std::cerr << "SCIM: Error - Invalid mode\n";
        return -4;
    }

    // information that will be specified by the arguments passed to the tool
    char *DiskImageFileName = NULL;
    bool DiskImageSpecified = false, TargetEntrySpecified = false;
    char *TargetEntryName = NULL;

    // - parse any other arguments
    // - i starts at 1 instead of 0 because argv[0] is the program name
    for(int i = 1; i < argc; i++) {
        // prevent the mode argument from being misinterpreted as anything else
        if(i == scim::MODE_INDEX) 
            continue;
        
        // specify the disk image file name
        if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--image")) {
            // make sure the argument is being used correctly
            if(i + 1 == argc || i + 1 == scim::MODE_INDEX ||
               DiskImageSpecified == true) {
                std::cerr << "SCIM: Error - Invalid usage of switch \"" << argv[i] << "\"\n";
                return -2;
            }
            DiskImageFileName = argv[++i];
            DiskImageSpecified = true;
        } else if(!strcmp(argv[i], "-e") || !strcmp(argv[i], "--entry")) {
            // make sure the argument is being used correctly
            if(i + 1 == argc || i + 1 == scim::MODE_INDEX ||
               TargetEntrySpecified == true) {
                std::cerr << "SCIM: Error - Invalid usage of switch \"" << argv[i] << "\"\n";
                return -2;
            }
            TargetEntryName = argv[++i];
            TargetEntrySpecified = true;
        }
    }

    // do mode independent operations such as opening the disk image

    // make sure a disk image was specified before SCIM tries to read it
    if(!DiskImageSpecified) {
        std::cerr << "SCIM: Error - Disk image not specified\n";
        return -3;
    }

    FILE *ImageReadStream = fopen(DiskImageFileName, "rb");
    if(!ImageReadStream) {
        std::cerr << "SCIM: Error - Could not open disk image\n";
        return -5;
    }

    // Initialise the file system
    scim::FAT12::Disk FileSystem;
    FileSystem.Initialise(ImageReadStream);

    // perform operations exclusive to the different modes

    scim::FAT::DirectoryEntry_t *TargetEntry;
    char *EntryBuffer = NULL;

    switch(mode) {
        case scim::M_LIST:
            for(int i = 0; i < FileSystem.FS_Info.BPB.RootDirectoryEntries; i++) {
                if(FileSystem.RootDirectory[i].Name[0] == 0) break;     // an entry with an empty name marks the end of the root directory
                printf("%.11s\n", FileSystem.RootDirectory[i].Name);    // I used printf instead of cout because i know how to only print 11 characters with printf but not with cout and I couldnt find anything online about it
            }
            break;
        case scim::M_READ:
            if(!TargetEntrySpecified) {
                std::cerr << "SCIM: Error - No target entry was specified\n";
                return -7;
            }
            TargetEntry = FileSystem.FindEntry(TargetEntryName);
            if(!TargetEntry) {
                std::cerr << "SCIM: Error - Could not find the file entry\n";
                return -8;
            }
            // very long line that just mallocs the size of the file entry rounded up to the next cluster
            EntryBuffer = (char *)malloc(TargetEntry->Size + (FileSystem.FS_Info.BPB.SectorsPerCluster * FileSystem.FS_Info.BPB.BytesPerSector - (TargetEntry->Size % (FileSystem.FS_Info.BPB.SectorsPerCluster * FileSystem.FS_Info.BPB.BytesPerSector))));
            if(!FileSystem.ReadEntry(ImageReadStream, TargetEntry, EntryBuffer)) {
                std::cerr << "SCIM: Error - Could not read the target entry\n";
                return -9;
            }
            std::cout << EntryBuffer;
            break;
        default:
            std::cerr << "SCIM: Error - Unexpected Mode\n";
            return -6;          // I don't think this error is possible to produce but better safe than sorry
    }

    return 0;

}
