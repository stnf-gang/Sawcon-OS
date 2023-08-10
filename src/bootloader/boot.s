# boot.s
#
# main source file for the boot sector of the SawconOS Bootloader
# Compiled using GNU as
# This file was written for the SawconOS Bootloader
# 
# Written: Monday 7th August 2023
# Last Updated: Thursday 10th August 2023
# 
# Written by Gabriel Jickells

.code16                                                             # the BIOS loads boot sector code in a 16 bit CPU mode. Some BIOSes start boot sector code in a 32 bit mode but it is standard for it to be loaded in 16 bit mode

print_char:
    mov $0x0e, %ah                                                  # teletype output
    mov $'a', %al                                                   # character to print
    mov $0, %bh                                                     # page number
    int $0x10                                                       # BIOS video functions

hang:
    cli                                                             # hardware interrupts can unhalt the CPU while they are handled so they should be disabled before halting the CPU
    hlt

.org 510                                                            # the last 2 bytes of the boot sector are used by the BIOS to check if a disk is bootable
.word 0xAA55                                                        # bytes 0x55, 0xAA are used as the BIOS boot signature at the end of the boot sector
