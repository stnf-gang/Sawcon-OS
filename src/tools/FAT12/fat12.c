// fat12.c
//
// main source file for the SCIM image manipulation tool
// This file was written for the SawconOS Host Tools
// 
// Written: Sunday 14th May 2023
// Last Updated: Thursday 18th May 2023
//
// Written by Gabriel Jickells

#include "fat12.h"

int main(int argc, char **argv) {
    printf("SawconOS Disk Image Tools \"SCIM\" v%s\n", SCIM_VERSION);
    // make sure arguments were passed to the tool
    if(argc == 1) {
        fprintf(stderr, "SCIM: Error E0001 - No Parameters\n");
        return -1;
    }
    // parse the arguments
    unsigned int mode; char *DiskName = NULL; char *File_IN = NULL;
    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i],"--help")) {
            for(int j = 0; HelpPage[j] != NULL; j++)
                printf("\t%s\n", HelpPage[j]);
                return 0;
        } else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--mode")) {
            if(i == argc - 1) {
                fprintf(stderr, "SCIM: Error E0002 - Invalid Argument Syntax\n");
                return -2;
            }
            i++;
            for(int j = 1; ModePage[j] != NULL; j++)
                if(!strcmp(argv[i], ModePage[j])) mode = j;
        } else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--disk")) {
            if(i == argc - 1) {
                fprintf(stderr, "SCIM: Error E0002 - Invalid Argument Syntax\n");
                return -2;
            }
            DiskName = argv[++i];
        } else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input")) {
            if(i == argc - 1) {
                fprintf(stderr, "SCIM: Error E0002 - Invalid Argument Syntax\n");
                return -2;
            }
            File_IN = argv[++i];
        } else {
            fprintf(stderr, "SCIM: Errir E0004 - Invalid Argument\n");
            return -4;
        }
    }
    if(mode == 0) {
        fprintf(stderr, "SCIM: Error E0003 - Invalid Mode\n");
        return -3;
    }
    if(DiskName == NULL) {
        fprintf(stderr, "SCIM: Error E0006 - Disk Image Not Specified\n");
        return -6;
    }
    FILE *DiskStream; BootRecord_t *d_BootRecord; DirectoryEntry_t *d_RootDirectory;
    DirectoryEntry_t *fs_Entry;
    switch(mode) {
        case M_READ:
            if(File_IN == NULL) {
                fprintf(stderr, "SCIM: Error E0009 - Input File Not Specified\n");
                return -9;
            }
            DiskStream = fopen(DiskName, "rb");
            if(DiskStream == NULL) {
                fprintf(stderr, "SCIM: Error E0005 - Could Not Open Disk Image \"%s\"\n", DiskName);
                return -5;
            }
            d_BootRecord = ReadBootRecord(DiskStream);
            if(d_BootRecord == NULL) {
                fprintf(stderr, "SCIM: Error E0007 - Could Not Read Boot Sector Of Disk Image\n");
                free(d_BootRecord);
                return -7;
            }
            d_RootDirectory = ReadRootDirectory(DiskStream, d_BootRecord, NULL);
            if(d_RootDirectory == NULL) {
                fprintf(stderr, "SCIM: Error E0008 - Could Not Read Root Directory Of Disk Image\n");
                free(d_BootRecord);
                free(d_RootDirectory);
                return -8;
            }
            fs_Entry = FindFile(d_BootRecord, d_RootDirectory, File_IN);
            if(fs_Entry == NULL) {
                fprintf(stderr, "SCIM: Error E000a - Could Not Open Input File\n");
                return -10;
            }
            break;
        case M_LIST:
            DiskStream = fopen(DiskName, "rb");
            if(DiskStream == NULL) {
                fprintf(stderr, "SCIM: Error E0005 - Could Not Open Disk Image \"%s\"\n", DiskName);
                return -5;
            }
            d_BootRecord = ReadBootRecord(DiskStream);
            if(d_BootRecord == NULL) {
                fprintf(stderr, "SCIM: Error E0007 - Could Not Read Boot Sector Of Disk Image\n");
                free(d_BootRecord);
                return -7;
            }
            d_RootDirectory = ReadRootDirectory(DiskStream, d_BootRecord, NULL);
            if(d_RootDirectory == NULL) {
                fprintf(stderr, "SCIM: Error E0008 - Could Not Read Root Directory Of Disk Image\n");
                free(d_BootRecord);
                free(d_RootDirectory);
                return -8;
            }
            printf("NAME\n");
            for(int i = 0; i < d_BootRecord->RootDirectoryEntries; i++) {
                if(d_RootDirectory[i].Name[0] == 0) break;
                printf("\"%.11s\"\n", d_RootDirectory[i].Name);
            }
            break;
    }
    free(d_BootRecord);
    free(d_RootDirectory);
    return 0;
}

BootRecord_t *ReadBootRecord(FILE* Disk) {
    BootRecord_t *returnVal = (BootRecord_t *)malloc(sizeof(BootRecord_t));
    if(fread(returnVal, sizeof(BootRecord_t), 1, Disk) <= 0) {
        free(returnVal);
        return NULL;
    }
    return returnVal;
}

bool ReadSectors(FILE *Disk, BootRecord_t *p_BootRecord, uint32_t LBA, uint32_t Count, void *BufferOut) {
    bool ok = true;
    ok &= fseek(Disk, LBA * p_BootRecord->BytesPerSector, SEEK_SET) == 0;
    ok &= fread(BufferOut, p_BootRecord->BytesPerSector, Count, Disk) > 0;
    return ok;
}

DirectoryEntry_t *ReadRootDirectory(FILE *Disk, BootRecord_t *p_BootRecord, uint32_t *DataSectionOut) {
    // calculate the size of the root directory and its location on the disk
    uint32_t RootDirectoryLBA = p_BootRecord->ReservedSectors + (p_BootRecord->SectorsPerFAT * p_BootRecord->TotalFATs);
    uint32_t RootDirectorySectors = (p_BootRecord->RootDirectoryEntries * sizeof(DirectoryEntry_t) + p_BootRecord->BytesPerSector - 1) / p_BootRecord->BytesPerSector;      // I used a trick to make the division always round up so all of the root directory will always be included
    if(DataSectionOut) *DataSectionOut = RootDirectoryLBA + RootDirectorySectors;
    DirectoryEntry_t *RootDirOut = (DirectoryEntry_t *)malloc(RootDirectorySectors * p_BootRecord->BytesPerSector);
    if(!ReadSectors(Disk, p_BootRecord, RootDirectoryLBA, RootDirectorySectors, RootDirOut)) {
        free(RootDirOut);
        return NULL;
    }
    return RootDirOut;
}

DirectoryEntry_t *FindFile(BootRecord_t *p_BootRecord, DirectoryEntry_t *Directory, char *Name) {
    for(int i = 0; i < p_BootRecord->RootDirectoryEntries; i++) {
        if(Directory[i].Name[0] == 0) return NULL;
        if(!memcmp(Directory[i].Name, Name, 11)) return &Directory[i];
    }
    return NULL;
}
