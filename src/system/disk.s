# disk.s
#
# source file for the disk SawconOS System Library
# Written for SawconOS System Alpha 1.0
# 
# Written: Sunday 8th October 2023
# Last Updated: Wednesday 11th October 2023
# 
# Written by Gabriel Jickells

.code16

.equ DISK_RESET, 0x00                   # mov $DISK_RESET, %ah ... int $DISK_FUNCTIONS
.equ DISK_FUNCTIONS, 0x13               # int $DISK_FUNCTIONS
.equ DISK_READSECTORS, 0x02             # mov $DISK_READSECTORS, %ah ... int $DISK_FUNCTIONS
.equ DISK_GETPARAMS, 0x08               # mov $DISK_GETPARAMS, %ah ... int $DISK_FUNCTIONS

.equ READSECT_SUCCESS, 0
.equ READSECT_FAIL, 1

.equ RESETDISK_SUCCESS, 0
.equ RESETDISK_FAIL, 1

.equ GETPARAMS_SUCCESS, 0
.equ GETPARAMS_FAIL, 1

.section .text

# int __cdecl ReadSectors(unsigned char _drive, unsigned short _LBA, unsigned char _count, unsigned char* __far _Buffer, Geometry_t _geometry)
.globl ReadSectors
ReadSectors:
    push %bp
    mov %sp, %bp
    push $0                             # unsigned short _cylinder
    push $0                             # unsigned short _head
    push $0                             # unsigned short _sector
    push %di                            # %di is not a scratch register
    push %es                            # %es will be destroyed but is not saved by pusha
    # BIOS does not support reading from disks with LBA values
    # equivalent C code: LBA2CHS(_LBA, &_cylinder, &_head, &_sectorm _geometry);
    pushw 14(%bp)                       # arg4 = _geometry
    lea -6(%bp), %ax                    # %ax = &_sector
    push %ax                            # arg3 = &_sector
    lea -4(%bp), %ax                    # %ax = &_head
    push %ax                            # arg2 = &_head
    lea -2(%bp), %ax                    # %ax = &_cylinder
    push %ax                            # arg1 = &_cylinder
    push 6(%bp)                         # arg0 = _LBA
    call LBA2CHS                        # LBA2CHS(arg0,arg1,arg2,arg3, arg4);
    add $2 * 5, %sp                     # remove the arguments from the stack
    mov $3, %di                         # store the retry count in %di
    ReadSectors.try:
        test %di, %di                   # check for any remaining retries
        jz ReadSectors.failure
        # read the sectors
        mov $DISK_READSECTORS, %ah
        mov 8(%bp), %al                 # %al = _count; // %al is the number of sectors to read
        mov -2(%bp), %ch                # %ch = _cylinder; // %ch is the low 8 bits of the cylinder number
        mov -4(%bp), %dh                # %dh = _head; // %dh is th head number
        # %cl stores the high 2 bits of the cylinder as well as the sector number
        mov -3(%bp), %cl                # get the high 8 bits of the cylinder number
        shl $6, %cl                     # put the lower 2 bits of this in the upper 2 bits of the register
        or -6(%bp), %cl                 # put the sector number with the cylinder number
        # get the output address
        mov 10(%bp), %bx                # %bx = _buffer.segment
        mov %bx, %es
        mov 12(%bp), %bx                # %bx = _buffer.offset
        # call the BIOS function
        stc                             # avoid some BIOS bugs
        int $DISK_FUNCTIONS
        mov $READSECT_SUCCESS, %ax      # using xor would affect the carry flag
        jnc ReadSectors.end
        # reset the disk system
        # equivalent C code: ResetDisk(_disk);
        push 4(%bp)                     # arg0 = _disk
        call ResetDisk                  # ResetDisk(arg0);
        add $2, %sp                     # remove the arguments from the stack
        cmp $RESETDISK_SUCCESS, %ax     # check the return value
        jne ReadSectors.failure         # not much more can be done if resetting the disk system doesn't work
        jmp ReadSectors.try
    ReadSectors.failure:
        mov $READSECT_FAIL, %ax
    ReadSectors.end:
        pop %es
        pop %di
        mov %bp, %sp                    # remove the local variables from the stack
        pop %bp
        ret

# int __cdecl ResetDisk(unsigned char _drive)
.globl ResetDisk
ResetDisk:
    push %bp
    mov %sp, %bp
    mov $DISK_RESET, %ah
    mov 4(%bp), %dl
    stc                                 # avoid some BIOS bugs
    int $DISK_FUNCTIONS
    jc ResetDisk.fail
    mov $RESETDISK_SUCCESS, %ax
    jmp ResetDisk.end
    ResetDisk.fail:
        mov $RESETDISK_FAIL, %ax
    ResetDisk.end:
        mov %bp, %sp
        pop %bp
        ret

# int __cdecl GetDiskGeometry(unsigned char _drive, Geometry_t *_geometryOut)
.globl GetDiskGeometry
GetDiskGeometry:
    push %bp
    mov %sp, %bp
    pusha                               # save any registers that might be destroyed
    push %es
    mov $DISK_GETPARAMS, %ah
    mov 4(%bp), %dl
    # %es:%di needs to zeroed out on some BIOSes to avoid bugs
    xor %di, %di
    mov %di, %es
    stc                                 # avoid bugs on some BIOSes
    int $DISK_FUNCTIONS
    jc GetDiskGeometry.fail
    # save the parameters in _GeometryOut
    movw 6(%bp), %si                    # %si = _geometryOut
    and $0x3f, %cx                      # remove the cylinder information from %cx and leave the sector information
    mov %cx, (%si)                      # _geometryOut->SectorsPerTrack = (unsigned short)MAX_SECTOR_NUMBER; // don't need to add 1 because sector numbers start at 1
    mov %dh, %cl                        # put the maximum head number in a word sized register with the upper half zeroed out
    inc %cl                             # Head count = maximum head number + 1 because head numbers start at 0
    mov %cx, 2(%si)                     # _geometryOut->Heads = (unsigned short)MAX_HEAD_NUMBER + 1;
    GetDiskGeometry.success:
        mov $GETPARAMS_SUCCESS, %ax
        jmp GetDiskGeometry.end
    GetDiskGeometry.fail:
        mov $GETPARAMS_FAIL, %ax
    GetDiskGeometry.end:
        pop %es
        popa                            # restore any registers that the caller might need
        mov %bp, %sp
        pop %bp
        ret

# void LBA2CHS(unsigned short _LBA, unsigned short *_cylinderOut, unsigned short *_headOut, unsigned short *_sectorOut, Geometry_t *_geometry)
.globl LBA2CHS
LBA2CHS:
    push %bp
    mov %sp, %bp
    # save non scratch registers
    push %si
    push %di
    mov 12(%bp), %si                    # %si = _geometry
    mov 4(%bp), %ax                     # %ax = _LBA
    xor %dx, %dx                        # div expects %ax to be empty
    divw (%si)                          # %ax = _LBA / _geometry->SectorsPerTrack; %dx = _LBA % _geometry->SectorsPerTrack;
    inc %dx                             # sector numbers start at 1 whereas remainders start at 0
    mov 10(%bp), %di                    # %di = _sectorOut
    mov %dx, (%di)                      # *_sectorOut = (_LBA % _geometry->SectorsPerTrack) + 1;
    xor %dx, %dx                        # prepare for the next division
    divw 2(%si)                         # %ax = (_LBA / _geometry->SectorsPerTrack) / _geometry->Heads; %dx = (_LBA / _geometry->SectorsPerTrack) % _geometry->Heads;
    mov 6(%bp), %di                     # %di = _cylinderOut;
    mov %ax, (%di)                      # *_cylinderOut = (_LBA / _geometry->SectorsPerTrack) / _geometry->Heads;
    mov 8(%bp), %di                     # %di = _headOut;
    mov %dx, (%di)                      # *headOut = (_LBA / _geometry->SectorsPerTrack) % _geometry->Heads;
    # restore non-scratch registers
    pop %di
    pop %si
    mov %bp, %sp
    pop %bp
    ret
