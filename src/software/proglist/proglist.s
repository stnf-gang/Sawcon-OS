# proglist.s
#
# main source file for proglist.bin
# proglist.bin will list all the files in the root directory of the boot drive then return to progload
# compiled using GNU as
#
# Written: Wednesday 11th October 2023
# Last Updated: Wednesday 11th October 2023
#
# Written by Gabriel Jickells

# make sure to link with the right system libraries
# set the include path to where the system libraries are
.include "misc_functions.inc"


.section .text
.code16

# these addresses are defined by progload
.equ ROOT_DIR_SEGMENT, 0x0000
.equ ROOT_DIR_OFFSET, 0x0700

.equ END_OF_DIR, 0
.equ FILENAME_LEN, 11
.equ SIZEOF_DIRECTORY_ENTRY, 32
.equ DELETED, 0xe5

.globl main
main:
    push %ds
    mov $ROOT_DIR_SEGMENT, %ax
    mov %ax, %ds
    mov $ROOT_DIR_OFFSET, %si
    main.loop:
        # check for special characters
        mov (%si), %ax
        cmp $END_OF_DIR, %ax
        je main.end
        cmp $DELETED, %ax
        je main.next
        # print the current file name
        push $FILENAME_LEN
        push %si
        call puts_len
        push $str_crlf_linefeed
        call puts
        add $6, %sp
        main.next:
            add $SIZEOF_DIRECTORY_ENTRY, %si
            jmp main.loop
    main.end:
        pop %ds
        retf                                            # return to progload

.section .data

str_crlf_linefeed: .asciz "\r\n"
