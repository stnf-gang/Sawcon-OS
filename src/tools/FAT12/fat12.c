// fat12.c
//
// main source file for the SCIM image manipulation tool
// This file was written for the SawconOS Host Tools
// 
// Written: Sunday 14th May 2023
// Last Updated: Wednesday 17th May 2023
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
    unsigned int mode; char *DiskName = NULL;
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
    FILE *DiskStream; BootRecord_t *d_BootRecord;
    switch(mode) {
        case M_READ:
            DiskStream = fopen(DiskName, "rb");
            if(DiskStream == NULL) {
                fprintf(stderr, "SCIM: Error E0005 - Could Not Open Disk Image \"%s\"\n", DiskName);
                return -5;
            }
            d_BootRecord = ReadBootRecord(DiskStream);
            if(d_BootRecord == NULL) {
                fprintf(stderr, "SCIM: Error E0007 - Could Not Read Boot Sector Of Disk Image\n");
                return -7;
            }
            break;
    }
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
    // todo
    return true;
}
