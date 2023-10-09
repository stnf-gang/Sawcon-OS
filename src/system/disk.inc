# disk.inc
# 
# includes all the external references for disk.o
# the program still needs to link with disk.o
# Written for use with assembly files
# Written for SawconOS System Alpha 1.0
#
# Written: Monday 9th October 2023
# Last Updated: Monday 9th October 2023
#
# Written by Gabriel Jickells

.equ READSECT_SUCCESS, 0
.equ READSECT_FAIL, 1

.equ RESETDISK_SUCCESS, 0
.equ RESETDISK_FAIL, 1

.equ GETPARAMS_SUCCESS, 0
.equ GETPARAMS_FAIL, 1

.extern ReadSectors                             # int __cdecl ReadSectors(unsigned char _drive, unsigned short _LBA, unsigned char _count, unsigned char* __far _Buffer, Geometry_t *geometry)
.extern ResetDisk                               # int __cdecl ResetDisk(unsigned char _drive)
.extern GetDiskGeometry                         # int __cdecl GetDiskGeometry(unsigned char _drive, Geometry_t *_geometryOut)
.extern LBA2CHS                                 # void __cdecl LBA2CHS(unsigned short _LBA, unsigned short *_cylinderOut, unsigned char *_headOut, unsigned short *_sectorOut, Geometry_t *_geometry)
