# boot.s
# 
# source file for the bootsector of SawconOS
# This file was written for the SawconOS Bootloader
# 
# Written: Thursday 11th May 2023
# Last Updated: Friday 12th May 2023
# 
# Written by Gabriel Jickells

.code16                                         # BIOS starts this code in 16 bit "real" mode

.equ MOD10h_WRITECHAR, 0x0e

# initialise the stack and segment registers
start:
    xor %ax, %ax                                # segment registers can only be set to the value of another register
    mov %ax, %ds                                # reset the data segment register
    mov %ax, %es                                # reset the "extra" segment register
    ljmp $0, $main                              # the code segment register should only be set with a far jump

main:
    mov $MSG_INTRO, %si
    call puts
    cli
    hlt

# ===Variable Data===

MSG_INTRO: .asciz "Sawcon Bootloader \"PEARL\" v0.0.00\n\r"

# ===Functions===

# ==puts==
# prints a NULL-Terminated "ASCIZ" string to the screen
# =ARGUMENTS=
# %si = (char *) pointer to string
# =OUTPUT=
# none
puts:
    pusha
    mov $MOD10h_WRITECHAR, %ah                  # %ah = int $0x10 mode
    puts.char:
        lodsb                                   # get the next character from (%si) into %al and increment %si
        test %al, %al                           # performs an AND operation without modifying any registers. If %al is 0 then the zero flag will be safe
        jz puts.end
        int $0x10                               # call the BIOS video services
        jmp puts.char
    puts.end:
        popa
        ret

.org 510
.word 0xaa55
