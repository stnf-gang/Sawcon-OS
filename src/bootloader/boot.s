# boot.s
#
# main source file for the boot sector of the SawconOS Bootloader
# Compiled using GNU as
# This file was written for the SawconOS Bootloader
# 
# Written: Monday 7th August 2023
# Last Updated: Friday 11th August 2023
# 
# Written by Gabriel Jickells

.code16                                 # the BIOS loads boot sector code in a 16 bit CPU mode. Some BIOSes start boot sector code in a 32 bit mode but it is standard for it to be loaded in 16 bit mode

.equ BOOT_ADDR, 0x7c00                  # address that the boot sector is expected to be loaded into. This address is standard for all BIOSes
.equ INT13_READSECT, 0x02               # value to put in %ah to read sectors from the disk using int 0x13
.equ INT10_TTY, 0x0e                    # value to put in %ah to use the teletype output function of int 0x10
.equ INT10_SETMODE, 0x00                # sets the video mode

# reset segment registers to known values and set up the stack
start:
    xor %ax, %ax                        # use the value in %ax to reset the segment registers to 0
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $BOOT_ADDR, %sp                 # start the stack at the beginning of the boot sector. The stack grows downwards so nothing will be overwritten
    mov %sp, %bp
    ljmp $0, $main                      # this far jump instruction makes sure the code segment is set to 0, which isn't standard for all BIOSes

# ==puts==
# prints a NULL terminated ASCII string to the screen using the BIOS
# this function was written for text modes and may act unexpectedly in graphical modes
# =ARGUMENTS=
# - %si = pointer to string
# =OUTPUT=
# none
puts:
    pusha                               # none of the registers will be used for output so they should all be saved before modifying their values
    mov $INT10_TTY, %ah
    mov $0, %bh                         # page number
    puts.loop:
        lodsb
        test %al, %al                   # check for end of string
        jz puts.end
        int $0x10
        jmp puts.loop                   # print the next character
    puts.end:
        popa
        ret

main:
    # make sure we are in the expected video mode
    mov $0x00, %ah
    mov $0x02, %al
    int $0x10
    # BIOS disk read test
    mov $INT13_READSECT, %ah
    mov $1, %al                         # amount of sectors to read
    mov $0, %ch                         # cylinder to read from
    mov $2, %cl                         # sector to read from
    mov $0, %dh                         # head to read from
    # dl is used to store the drive number. The bios puts the boot disk number into dl as the default
    xor %bx, %bx                        # %es can only be set to the value of another register. %bx is fine to use here since is value will be set to something else after
    mov %bx, %es                        # segment to read into
    mov $sector2_str, %bx               # offset to read into
    int $0x13                           # BIOS disk functions
    # check if the data was actually read
    mov $sector2_str, %si
    call puts

hang:
    cli                                 # hardware interrupts can unhalt the CPU while they are handled so they should be disabled before halting the CPU
    hlt

.org 510                                # the last 2 bytes of the boot sector are used by the BIOS to check if a disk is bootable
.word 0xAA55                            # bytes 0x55, 0xAA are used as the BIOS boot signature at the end of the boot sector

# this section of the code will be in the second sector of the disk
sector2_str: .asciz "Success! \02\n\r"
