# boot.s
#
# main source file for the boot sector of the SawconOS Bootloader
# Compiled using GNU as
# This file was written for SawconOS Bootloader Alpha 1.0
# 
# Written: Monday 7th August 2023
# Last Updated: Wednesday 20th September 2023
# 
# Written by Gabriel Jickells

.code16                                 # the BIOS loads boot sector code in a 16 bit CPU mode. Some BIOSes start boot sector code in a 32 bit mode but it is standard for it to be loaded in 16 bit mode

.equ BOOT_ADDR, 0x7c00                  # address that the boot sector is expected to be loaded into. This address is standard for all BIOSes
.equ EXPECTED_CS, 0                     # the code segment we will be operating in
.equ DISK_READSECT, 0x02                # mov $DISK_READSECT, %ah ... int $DISK_FUNCTIONS
.equ VIDEO_TTY, 0x0e                    # mov $VIDEO_TTY, %ah ... int $VIDEO_FUNCTIONS
.equ VIDEO_SETMODE, 0x00                # mov $VIDEO_SETMODE, %ah ... int $VIDEO_FUNCTIONS
.equ DISK_GETPARAMS, 0x08               # mov $DISK_GETPARAMS ... int $DISK_FUNCTIONS
.equ DISK_FUNCTIONS, 0x13               # int $DISK_FUNCTIONS
.equ VIDEO_FUNCTIONS, 0x10              # int $VIDEO_FUNCTIONS
.equ VGA_TEXT_80x25_16COLOUR, 0x02      # mov $VIDEO_SETMODE, %ah; mov $VGA_TEXT_80x25_16COLOUR, %al ... int $VIDEO_FUNCTIONS
.equ DISK_RESET, 0x00                   # mov $DISK_RESET, %ah ... int $DISK_FUNTIONS
.equ FAT_ENTRY_SIZE, 32                 # measured in bytes
.equ FIRST_CLUSTER_LOW_OFFSET, 26       # byte offset of the low first cluster value in a directory entry
.equ CLUSTER_END, 0xff8                 # do not try to read this cluster or anything above it
.equ STAGE2_LOCATION, 0x8e00            # load the stage 2 binary into this address

# FAT Header
# BIOS Parameter Block (BPB)
DataSkip:
jmp start
nop
OEM_Identifier: .ascii "SAWCONOS"       # name of the manufacturer or tool that formatted the disk
BytesPerSector: .word 512
SectorsPerCluster: .byte 1              # higher values make files take up more space but make the FAT able to hold more information
ReservedSectors: .word 1                # effectively the LBA of the FAT. Refers to the number of sectors reserved for the bootloader at the start of the disk
TotalFATs: .byte 1                      # all FATs on the disk are identical and are used for disk repair and verification
RootDirectoryEntries: .word 128         # amount of file entries that can fit in the root directory
TotalSectors: .word 2880                # 1.44MiB disk
MediaDescriptor: .byte 0xF0             # used in old versions of MS DOS to refer to a 1.44MiB floppy
SectorsPerFAT: .word 8                  # 4KiB reserved for each FAT. This equates to about 2730 clusters which doesnt use the entirety of the disk space but the remainder can be reserved for other purposes
SectorsPerTrack: .word 18               # value should be updated to the values returned by int $0x13 at startup
TotalHeads: .word 2                     # value should be updated to the values returned by int $0x13 at startup
HiddenSectors: .long 0                  # effectively the LBA of the current partition. Refers to the number of sectors before the boot sector
LargeSectors: .long 0                   # used instead of total sectors if there are more than 65535 sectors on the disk
# Extended BIOS Parameter Block (EBPB)
DriveNumber: .byte 0x00                 # should be replaced with the value in %dl at startup
_reserved: .byte 0                      # used in windows NT
Signature: .byte 0x28                   # tells the driver that this is a valid file system. Must be 0x28 or 0x29
SerialID: .long 0x23A1005C              # Serial number of the disk
VolumeLabel: .ascii "SAWCON  100"       # name of the disk
SystemID: .ascii "FAT12   "             # specify the file system

# reset segment registers to known values and set up the stack
start:
    xor %ax, %ax                        # use the value in %ax to reset the segment registers to 0
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $BOOT_ADDR, %sp                 # start the stack at the beginning of the boot sector. The stack grows downwards so nothing will be overwritten
    mov %sp, %bp
    ljmp $EXPECTED_CS, $main            # this far jump instruction makes sure the code segment is set to 0, which isn't standard for all BIOSes

# LBA2CHS
# converts a 16 bit LBA value into a CHS value that can be used with the BIOS
# =ARGUMENTS=
# - %ax = LBA
# =OUTPUT=
# - %ch = Low 8 bits of Cylinder number
# - %dh = Head number
# - %cl = Sector number + (High 2 bits of Cylinder number << 6)
LBA2CHS:
    push %ax
    push %dx
    xor %dx, %dx                        # div instruction expects %dx to be 0
    divw (SectorsPerTrack)              # all conversion operations use this division
    inc %dx                             # the remainder (which will be used for the sector number) is in %dx. Sector numbers start at 1 and LBA numbers start at 0 so the offset is taken care of here
    mov %dl, %cl                        # store the sector number in its expected location
    xor %dx, %dx                        # prepare for the next division
    divw (TotalHeads)                   # the Cylinder and Head conversion operations both use this division
    shl $6, %ah                         # move the high 2 bits of the cylinder number into the upper 2 bits of the register
    mov %al, %ch                        # store the cylinder number in its expected location
    or %ah, %cl                         # store the high 2 bits of the cylinder number in their expected location
    mov %dl, %al                        # save the head number
    pop %dx                             # restore the drive number in %dl
    mov %al, %dh                        # store the head numbner in its expected location
    pop %ax
    ret

# ReadSectors
# reads some number of sectors from a drive
# =ARGUMENTS=
# - %ax = LBA
# - %es:%bx = output address
# - %cl = number of sectors to read
# - %dl = drive to read from
# =OUTPUT=
# none
ReadSectors:
    pusha
    push %cx                            # save the number of sectors to read
    call LBA2CHS                        # BIOS expects the location on disk to be in CHS format
    pop %ax                             # put the number of sectors to read in its expected location
    mov $DISK_READSECT, %ah
    stc                                 # some BIOSes don't set the carry flag properly so it is done manually
    int $DISK_FUNCTIONS
    jc hang
    popa
    ret

# Cluster2LBA
# convert a FAT cluster number to an LBA address
# requires the root directory to have been read
# =ARGUMENTS=
# - %ax = Cluster
# =OUTPUT=
# - %ax = LBA
Cluster2LBA:
    sub $2, %ax                         # cluster numbers start at 2 instead of 0
    mulb (SectorsPerCluster)
    addw (DataSectionLBA), %ax          # clusters start counting data from the start of the data section
    ret

main:
    # save the boot drive number in the fat12 header
    movb %dl, (DriveNumber)
    # get the geometry of the boot drive
    mov $DISK_GETPARAMS, %ah
    # this function expects the drive number to be in %dl
    # Conveniently, the BIOS puts the boot drive into %dl at startup
    # to prevent BIOS bugs, %es:%di is set to 0:0
    xor %di, %di                        # usually, %di would be copied into %es but that was zeroed out in start to I dont need to do it again
    stc                                 # some BIOSes don't correctly set the carry flag so it is set manually here to be cleared on success
    int $DISK_FUNCTIONS
    jc hang                             # if GETPARAMS fails then we cannot continue
    # store the output of GETPARAMS in the fat12 header
    and $0x3f, %cx                      # lower 6 bits store the SectorsPerTrack value and the rest is the highest cylinder number. Only the sector is needed so the sector is zeroed out
    movw %cx, (SectorsPerTrack)
    inc %dh                             # head number starts at 0 and head count starts at 1
    mov %dh, %cl                        # %ch is already zeroed out
    movw %cx, (TotalHeads)
    # clear any information that the BIOS stored in %es:%di
    xor %di, %di
    mov %di, %es
    # make sure the computer is in the expected video mode
    mov $VIDEO_SETMODE, %ah
    mov $VGA_TEXT_80x25_16COLOUR, %al
    int $VIDEO_FUNCTIONS
    ReadRootDirectory:
        # calculate the root directory lba (SectorsPerFAT * TotalFATS + ReservedSectors)
        movw (SectorsPerFAT), %ax
        mulb (TotalFATs)
        addw (ReservedSectors), %ax
        push %ax                        # save the lba for after the sector count is calculated
        # calculate the number of sectors in the root directory (((RootDirectoryEntries * FAT_ENTRY_SIZE) + (BytesPerSector - 1)) / BytesPerSector)
        movw (RootDirectoryEntries), %ax
        mov $FAT_ENTRY_SIZE, %cl
        mul %cl
        addw (BytesPerSector), %ax
        dec %ax
        xor %dx, %dx
        divw (BytesPerSector)
        mov %ax, %cx                    # move the sector count into its expected location
        # read the sectors into the buffer
        pop %ax                         # put the lba into its expected location
        mov $buffer, %bx                # %es is already zeroed out
        movb (DriveNumber), %dl
        call ReadSectors
        # store the data section lba for use in the cluster 2 lba conversion
        add %cx, %ax
        movw %ax, (DataSectionLBA)
    FindEntry:
        mov $buffer, %si                # memory to start comparing from
        mov $11, %cx                    # number of characters to compare
        mov $0, %dx                     # use %dx to check that FindEntry hasn't gone out of the root directory
        FindEntry.loop:
            cmpw (RootDirectoryEntries), %dx
            je hang                     # the file hasn't been found
            mov $11, %cx
            mov $FileName, %di
            push %si                    # save current position in the root directory
            repe cmpsb
            pop %si
            je FindEntry.exit
            add $FAT_ENTRY_SIZE, %si    # go to the next entry in the root directory
            inc %dx                     # increment the index
            jmp FindEntry.loop
        FindEntry.exit:
            # get the first cluster of the entry
            add $FIRST_CLUSTER_LOW_OFFSET, %si
            movw (%si), %ax             # put the first cluster number in its expected location
            push %ax
    ReadFAT:
        movw (ReservedSectors), %ax
        mov $buffer, %bx
        movw (SectorsPerFAT), %cx
        movb (DriveNumber), %dl
        call ReadSectors
    ReadEntry:
        mov $STAGE2_LOCATION, %bx
        ReadEntry.loop:
            # while (currentCluster < CLUSTER_END)
            pop %ax                     # retrieve the current cluster from the stack
            cmp $CLUSTER_END, %ax
            jae ReadEntry.end
            # read the current cluster
            push %ax                    # save the current cluster
            call Cluster2LBA            # get the LBA to read from
            movb (SectorsPerCluster), %cl
            call ReadSectors
            # calculate the next memory location to read into
            movw (BytesPerSector), %ax
            xor %ch, %ch
            mul %cx
            shr $4, %ax
            mov %es, %dx
            add %ax, %dx
            mov %dx, %ax
            mov %dx, %es
            # calculate the fat index of the next cluster
            pop %ax                     # get the current cluster from the stack
            # calculate the fat index (current cluster * 3 / 2)
            mov $3, %cx
            mul %cx
            mov $2, %cx
            xor %dx, %dx
            div %cx
            # get the value from the FAT
            mov $buffer, %si
            add %ax, %si
            mov (%si), %ax
            # remove the extra 4 bits
            test %dl, %dl               # get the remainder
            jz even
            odd:
                shr $4, %ax
            even:
                and $0xfff, %ax
                push %ax
            jmp ReadEntry.loop
    ReadEntry.end:
        ljmp $0, $0x8e00
    hang:
        cli                             # stop hardware interrupts from unhalting the cpu
        hlt

DataSectionLBA: .word 0x0000
FileName: .ascii "PROGLOADBIN"

.org 510                                # the last 2 bytes of the boot sector are used by the BIOS to check if a disk is bootable
.word 0xAA55                            # bytes 0x55, 0xAA are used as the BIOS boot signature at the end of the boot sector
buffer:
