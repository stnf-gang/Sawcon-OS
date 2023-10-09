# misc_functions.s
#
# source file for the misc_functions SawconOS System Library
# Written for SawconOS System Alpha 1.0
# 
# Written: Tuesday 3rd October 2023
# Last Updated: Saturday 7th October 2023
# 
# Written by Gabriel Jickells

.code16

.equ VIDEO_TTY, 0x0e                    # mov $VIDEO_TTY, %ah ... int $VIDEO_FUNCTIONS
.equ VIDEO_FUNCTIONS, 0x10              # int $VIDEO_FUNCTIONS
.equ SIZEOF_WORD, 2                     # measured in bytes

.section .text

# void __cdecl putc(char __c)
.globl putc
putc:
    push %bp
    mov %sp, %bp
    pusha
    mov $VIDEO_TTY, %ah
    xor %bx, %bx                        # set the page number to zero
    mov 4(%bp), %al                     # character to print goes in %al
    int $VIDEO_FUNCTIONS
    popa
    mov %bp, %sp
    pop %bp
    ret

# void __cdecl puts(char *__s)
.globl puts
puts:
    push %bp
    mov %sp, %bp
    pusha
    mov 4(%bp), %si
    puts.loop:
        lodsb                           # get the current character in the string
        test %al, %al                   # check for NULL terminator
        jz puts.end
        push %ax                        # pass the current character to putc
        call putc
        add $SIZEOF_WORD, %sp           # clean the stack
        jmp puts.loop
    puts.end:
        popa
        mov %bp, %sp
        pop %bp
        ret
