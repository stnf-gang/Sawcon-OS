# progload.s
# 
# main source file for the SawconOS Program Loader
# This file was written for SawconOS System Alpha 1.0
#
# Written: Tuesday 3rd October 2023
# Last Updated: Wednesday 11th October 2023
#
# Written by Gabriel Jickells

/*

    Dependencies (Link with these)
    - sc_system_libs/misc_funtcions.o
    - progload_libs/signature.o
    - sc_system_libs/disk.o

*/

.code16                                     # the bootloader loads this code in 16 bit real mode

# add the system libs path to the assembler with -I
.include "misc_functions.inc"
.include "progload/signature.inc"
.include "disk.inc"

.equ SIZEOF_WORD, 2                         # measured in bytes

.equ SIZEOF_ROOTDIRENTRY, 32

.equ BOOTSECTOR_SEGMENT, 0x0000
.equ BOOTSECTOR_OFFSET, 0x0500
.equ ROOT_DIR_SEGMENT, 0x0000
.equ ROOT_DIR_OFFSET, 0x0700
.equ FAT_SEGMENT, 0x0000
.equ FAT_OFFSET, 0x2000
.equ PROG_SEGMENT, 0x0000
.equ PROG_OFFSET, 0x5000

.equ BPB_BYTESPERSECTOR_OFFSET, 11
.equ BPB_SECTORSPERCLUSTER_OFFSET, 13
.equ BPB_RESERVEDSECTS_OFFSET, 14
.equ BPB_TOTALFATS_OFFSET, 16
.equ BPB_ROOTDIRENTRIES_OFFSET, 17
.equ BPB_SECTORSPERFAT_OFFSET, 22

.equ FIRST_CLUSTER_LOW_OFFSET, 26

.equ KB_FUNCTIONS, 0x16                     # int $KB_FUNCTIONS
.equ KB_READKEY, 0x00                       # mov $KB_READKEY, %ah ... int $KB_FUNCTIONS

.section .text

.globl entry
entry:
    mov %dl, (g_bootDrive)                  # save the boot drive in a global variable
    pushw $str_intro                        # pass the arguments
    call puts
    add $SIZEOF_WORD, %sp                   # clean the stack
    # Check the bootloader signature
    pushw $str_CheckBootSignature
    call puts
    add $SIZEOF_WORD, %sp
    # pass the signature in %ax and %bx to the function
    push %ax
    push %bx
    call checkBootSignature
    add $SIZEOF_WORD * 2, %sp
    cmp $CBS_ALPHA_SIGNATURE, %ax
    je AlphaSignature
    cmp $CBS_BETA_SIGNATURE, %ax
    je BetaSignature
    cmp $CBS_FUTURE_SIGNATURE, %ax
    je FutureSignature
    cmp $CBS_NOT_5C_SIGNATURE, %ax
    je UnofficialSignature
    cmp $CBS_OLD_SIGNATURE, %ax
    je OutdatedSignature
    cmp $CBS_VALID_SIGNATURE, %ax
    je afterSignature
    AlphaSignature:
        pushw $str_AlphaSignature
        call puts
        add $SIZEOF_WORD, %sp
        jmp afterSignature
    BetaSignature:
        pushw $str_BetaSignature
        call puts
        add $SIZEOF_WORD, %sp
        jmp afterSignature
    FutureSignature:
        pushw $str_FutureSignature
        call puts
        add $SIZEOF_WORD, %sp
        jmp afterSignature
    UnofficialSignature:
        pushw $str_UnofficialSignature
        call puts
        add $SIZEOF_WORD, %sp
        jmp afterSignature
    OutdatedSignature:
        pushw $str_OutdatedSignature
        call puts
        add $SIZEOF_WORD, %sp
    afterSignature:
        pushw $str_GetGeometry
        call puts
        add $SIZEOF_WORD, %sp
        # get the disk geometry
        pushw $g_BootDriveGeometry
        xor %ax, %ax
        movb (g_bootDrive), %al
        push %ax
        call GetDiskGeometry
        add $SIZEOF_WORD * 2, %sp
        # check for errors
        cmp $GETPARAMS_FAIL, %ax
        jne entry.ReadBootRecord
        push $str_NoGeometry
        call puts
        add $SIZEOF_WORD, %sp
        jmp entry.hang
    entry.ReadBootRecord:
        push $str_ReadBootRecord
        call puts
        add $SIZEOF_WORD, %sp
        # pass the parameters to ReadSectors
        pushw $g_BootDriveGeometry
        pushw $BOOTSECTOR_OFFSET
        pushw $BOOTSECTOR_SEGMENT
        pushw $1                                # 1 boot sector
        pushw $0                                # boot sector starts at the beginning of the disk
        pushw (g_bootDrive)
        call ReadSectors
        add $SIZEOF_WORD * 6, %sp
        cmp $READSECT_SUCCESS, %ax
        je entry.ReadRootDirectory
        push $str_NoBootRecord
        call puts
        add $SIZEOF_WORD, %sp
        jmp entry.hang
    entry.ReadRootDirectory:
        push $str_ReadRootDirectory
        call puts
        add $SIZEOF_WORD, %sp
        # set up the call to ReadSectors
        pushw $g_BootDriveGeometry
        pushw $ROOT_DIR_OFFSET
        pushw $ROOT_DIR_SEGMENT
        # calculate the number of sectors in the root directory
        movw (BOOTSECTOR_OFFSET + BPB_ROOTDIRENTRIES_OFFSET), %ax
        mov $SIZEOF_ROOTDIRENTRY, %cx
        mul %cx                                 # root_directory_bytes = BootRecord.BPB.RootDirEntries * sizeof(RootDirectoryEntry_t)
        # trick to round up the division: add the divisor - 1 to the dividend
        add (BOOTSECTOR_OFFSET + BPB_BYTESPERSECTOR_OFFSET), %ax
        dec %ax
        xor %dx, %dx                            # div expects %dx to be empty
        divw (BOOTSECTOR_OFFSET + BPB_BYTESPERSECTOR_OFFSET)
        push %ax                                # %ax now contains the sector count
        movw %ax, (g_DataSectionLBA)            # save for when the LBA is calculated
        # calculate the LBA of the root directory
        # since the count was the size of a byte, %ah is already empty
        movb (BOOTSECTOR_OFFSET + BPB_TOTALFATS_OFFSET), %al
        mulw (BOOTSECTOR_OFFSET + BPB_SECTORSPERFAT_OFFSET)
        addw (BOOTSECTOR_OFFSET + BPB_RESERVEDSECTS_OFFSET), %ax
        push %ax                                # %ax = bootrecord.totalfats * bootrecord.sectorsPerFAT + bootrecord.reservedSectors
        mov (g_DataSectionLBA), %bx
        add %bx, %ax
        mov %ax, (g_DataSectionLBA)
        # get the boot drive
        xor %ax, %ax
        mov (g_bootDrive), %al
        push %ax
        call ReadSectors
        add $SIZEOF_WORD * 6, %sp
        cmp $READSECT_SUCCESS, %ax
        je entry.ReadFAT
        push $str_NoRootDirectory
        call puts
        add $SIZEOF_WORD, %sp
        jmp entry.hang
    entry.ReadFAT:
        push $str_ReadFAT
        call puts
        add $SIZEOF_WORD, %sp
        # read the sectors in the FAT
        push $g_BootDriveGeometry
        push $FAT_OFFSET
        push $FAT_SEGMENT
        pushw (BOOTSECTOR_OFFSET + BPB_SECTORSPERFAT_OFFSET)
        pushw (BOOTSECTOR_OFFSET + BPB_RESERVEDSECTS_OFFSET)
        pushw (g_bootDrive)
        call ReadSectors
        add $SIZEOF_WORD * 6, %sp
        cmp $READSECT_SUCCESS, %ax
        je entry.command
        push $str_NoFAT
        call puts
        add $SIZEOF_WORD, %sp
        jmp entry.hang
    entry.command:
        push $str_prompt
        call puts
        add $SIZEOF_WORD, %sp
        mov $g_KB_Buffer, %di               # %di is used to store the index into the keyboard buffer 
        entry.command.loop:
            cmp $g_KB_Buffer + 11, %di      # 11 is the length of a FAT file name
            je entry.command.find
            mov $0, %ah
            int $0x16
            stosb                           # store the current byte in the buffer
            push %ax
            call putc
            add $SIZEOF_WORD, %sp
            jmp entry.command.loop
        entry.command.find:
            push $str_crlf_linefeed
            call puts
            add $SIZEOF_WORD, %sp
            # find the file that was specified
            push $g_KB_Buffer
            push $0                         # kb buffer segment
            call FindFile
            add $SIZEOF_WORD * 2, %sp
            cmp $0, %ax
            jne entry.command.execute       # should be replaced with code to read the file
            push $str_NoFileH
            call puts
            push $g_KB_Buffer
            call puts
            push $str_NoFileL
            call puts
            add $SIZEOF_WORD * 3, %sp
            jmp entry.command               # get the next command
        entry.command.execute:
            pushw (g_bootDrive)
            pushw %ax                       # pointer to entry to read
            call ReadEntry
            add $SIZEOF_WORD * 2, %sp
            cmp $0, %ax
            je entry.command.execute.success
            push $str_NoExecute
            call puts
            add $SIZEOF_WORD, %sp
            jmp entry.command
            entry.command.execute.success:
                lcall $PROG_SEGMENT, $PROG_OFFSET
                jmp entry.command
    entry.hang:
        cli
        hlt

# DirectoryEntry_t* __cdecl FindFile(char* __far Name)
# returns NULL when file is not found
FindFile:
    push %bp
    mov %sp, %bp
    push %si
    push %di
    push %ds
    push %es
    # store the part of the root directory we are comparing in %ds:%si
    mov $ROOT_DIR_SEGMENT, %ax
    mov %ax, %ds
    mov $ROOT_DIR_OFFSET, %si
    # store the file name we are comparing in %es:%di
    mov 4(%bp), %ax
    mov %ax, %es
    # store the current index in the root directory in %dx
    xor %dx, %dx                            # start at 0
    FindFile.loop:
        cmp (BOOTSECTOR_OFFSET + BPB_ROOTDIRENTRIES_OFFSET), %dx
        je FindFile.NoFile
        push %si
        mov $11, %cx                        # number of bytes to compare
        mov 6(%bp), %di
        repe cmpsb
        pop %si
        je FindFile.Found
        add $SIZEOF_ROOTDIRENTRY, %si       # go to the next entry
        inc %dx
        jmp FindFile.loop
    FindFile.Found:
        mov %si, %ax
        jmp FindFile.end
    FindFile.NoFile:
        mov $0, %ax
    FindFile.end:
        pop %es
        pop %ds
        pop %di
        pop %si
        mov %bp, %sp
        pop %bp
        ret

# int __cdecl ReadSectors(DirectoryEntry_t *Entry, char disk)
# returns 0 for success and 1 for failure
ReadEntry:
    push %bp
    mov %sp, %bp
    mov 4(%bp), %si
    mov FIRST_CLUSTER_LOW_OFFSET(%si), %ax # ax = Entry->FirstClusterLow
    ReadEntry.loop:
        cmp $0xff8, %ax
        jae ReadEntry.success
        push %ax                                        # save the current cluster number
        # read the current cluster
        push $g_BootDriveGeometry
        push $PROG_OFFSET
        push $PROG_SEGMENT
        pushw (BOOTSECTOR_OFFSET + BPB_SECTORSPERCLUSTER_OFFSET)
        # convert the cluster number to an LBA address
        push %ax
        call Cluster2LBA
        add $SIZEOF_WORD, %sp
        # pass the rest of the arguments
        push %ax                                        # %ax contains the return value
        push 6(%bp)                                     # disk
        call ReadSectors
        add $SIZEOF_WORD * 6, %sp
        # check the return value
        cmp $READSECT_SUCCESS, %ax
        jne ReadEntry.failure
        # calculate the next address to read into
        movw (BOOTSECTOR_OFFSET + BPB_BYTESPERSECTOR_OFFSET), %ax
        movw (BOOTSECTOR_OFFSET + BPB_SECTORSPERCLUSTER_OFFSET), %cx
        xor %ch, %ch
        mul %cx
        shr $4, %ax
        mov %es, %dx
        add %ax, %dx
        mov %dx, %es
        # calculate the fat index of the next cluster (CurrentCluster * 3 / 2)
        pop %ax
        mov $3, %cx
        mul %cx
        mov $2, %cx
        xor %dx, %dx                                    # empty %dx for div
        div %cx
        # get the value from the FAT
        mov $FAT_OFFSET, %si
        add %ax, %si
        mov (%si), %ax
        # remove the extra four bits to make the 16 bit %ax hold a 12 bit value
        test %dl, %dl
        jz ReadEntry.even
        ReadEntry.odd:
            shr $4, %ax
        ReadEntry.even:
            and $0xfff, %ax
            jmp ReadEntry.loop
    ReadEntry.success:
        mov $0, %ax
        jmp ReadEntry.end
    ReadEntry.failure:
        mov $1, %ax
    ReadEntry.end:
        mov %bp, %sp
        pop %bp
        ret

# unsigned short Cluster2LBA(unsigned short cluster)
Cluster2LBA:
    push %bp
    mov %sp, %bp
    mov 4(%bp), %ax
    sub $2, %ax
    mulb (BOOTSECTOR_OFFSET + BPB_SECTORSPERCLUSTER_OFFSET)
    addw (g_DataSectionLBA), %ax
    mov %bp, %sp
    pop %bp
    ret

.section .data

str_prompt: .asciz "\n\r[PROGLOAD]$"
str_crlf_linefeed: .asciz "\n\r"

str_intro: .asciz "SawconOS Progload For System Alpha 1.0\n\r"
str_CheckBootSignature: .asciz "Checking the bootloader signature...\n\r"
str_GetGeometry: .asciz "Getting the geometry of the boot drive...\n\r"
str_ReadBootRecord: .asciz "Reading the boot record of the boot disk...\n\r"
str_ReadRootDirectory: .asciz "Reading the root directory of the boot disk...\n\r"
str_ReadFAT: .asciz "Reading the FAT of the boot disk...\n\r"

str_NoGeometry: .asciz "!ERROR! - Could not get the geometry of the boot drive\n\r"
str_NoBootRecord: .asciz "!ERROR! - Could not read boot record\n\r"
str_NoRootDirectory: .asciz "!ERROR! - Could not read root directory\n\r"
str_NoFileH: .asciz "!ERROR! - Could not find file \""
str_NoFileL: .asciz "\"\n\r"
str_NoFAT: .asciz "!ERROR! - Could not read the FAT\n\r"
str_NoExecute: .asciz "!ERROR! - Could not read the file\n\r"

str_OutdatedSignature: .asciz "!WARNING! - Outdated version of the bootloader detected\n\r"
str_AlphaSignature: .asciz "!WARNING! - Alpha version of the bootloader detected. Possible stability issues\n\r"
str_BetaSignature: .asciz "!WARNING! - Beta version of the bootloader detected. Possible stability issues\n\r"
str_FutureSignature: .asciz "!WARNING! - Future version of the bootloader detected\n\r"
str_UnofficialSignature: .asciz "!WARNING! - Non-Sawcon Bootloader detected\n\r"

g_bootDrive: .byte 0x00
g_BootDriveGeometry:
    g_BootDriveGeometry.SectorsPerTrack: .word 0x0000
    g_BootDriveGeometry.Heads: .word 0x0000
g_KB_Buffer:
    .rept 11
    .byte 0x00
    .endr
    .byte 0                                 # NULL terminator for printing
g_DataSectionLBA: .word 0x0000
