# progload.s
# 
# main source file for the SawconOS Program Loader
# This file was written for SawconOS System Alpha 1.0
#
# Written: Tuesday 3rd October 2023
# Last Updated: Friday 6th October 2023
#
# Written by Gabriel Jickells

/*

    Dependencies (Link with these)
    - sc_system_libs/misc_funtcions.o
    - progload_libs/signature.o

*/

.code16                                     # the bootloader loads this code in 16 bit real mode

# add the system libs path to the assembler with -I
.include "misc_functions.inc"
.include "progload/signature.inc"
.include "disk.inc"

.equ SIZEOF_WORD, 2                         # measured in bytes

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
    entry.ReadBootRecord:
        push $str_ReadBootRecord
        call puts
        add $SIZEOF_WORD, %sp
    entry.hang:
        cli
        hlt

.section .data

str_intro: .asciz "SawconOS Progload For System Alpha 1.0\n\r"
str_CheckBootSignature: .asciz "Checking the bootloader signature...\n\r"
str_GetGeometry: .asciz "Getting the geometry of the boot drive\n\r"
str_NoGeometry: .asciz "!Error! - Could not get the geometry of the boot drive\n\r"
str_ReadBootRecord: .asciz "Reading the boot record of the boot disk\n\r"

str_OutdatedSignature: .asciz "!WARNING! - Outdated version of the bootloader detected\n\r"
str_AlphaSignature: .asciz "!WARNING! - Alpha version of the bootloader detected. Possible stability issues\n\r"
str_BetaSignature: .asciz "!WARNING! - Beta version of the bootloader detected. Possible stability issues\n\r"
str_FutureSignature: .asciz "!WARNING! - Future version of the bootloader detected\n\r"
str_UnofficialSignature: .asciz "!WARNING! - Non-Sawcon Bootloader detected\n\r"

g_bootDrive: .byte 0x00
g_BootDriveGeometry:
    g_BootDriveGeometry.SectorsPerTrack: .word 0x0000
    g_BootDriveGeometry.Heads: .word 0x0000
