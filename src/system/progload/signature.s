# signature.s
#
# functions and utilities for checking the bootloader signature
# Written for SawconOS System Alpha 1.0 running on Bootloader Alpha 1.02
#
# Written: Wednesday 4th October 2023
# Last Updated: Friday 6th October 2023
# 
# Written by Gabriel Jickells

.code16

.section .text

# CheckBootSignature function returns
.equ CBS_NOT_5C_SIGNATURE, 3
.equ CBS_ALPHA_SIGNATURE, 4
.equ CBS_BETA_SIGNATURE, 5
.equ CBS_NOT_VALID_SIGNATURE, 3
.equ CBS_FUTURE_SIGNATURE, 2
.equ CBS_OLD_SIGNATURE, 1
.equ CBS_VALID_SIGNATURE, 0

# int checkBootSignature(unsigned long Signature)
.globl checkBootSignature
checkBootSignature:
    push %bp
    mov %sp, %bp
    # check if the signature is a 5awCon signature
    checkBootSignature.isSC_Signature:
        mov 4(%bp), %ax                             # get the low 8 bits
        cmp $0x5C, %al                              # 0x5C at the end indicates a 5awCon signature
        je checkBootSignature.version
        mov $CBS_NOT_5C_SIGNATURE, %ax
        jmp checkBootSignature.end
    # check the version of the signature
    checkBootSignature.version:
        mov 6(%bp), %bx                             # get the upper 8 bits
        movw (CurrentBootSignature + 2), %ax        # get the upper 8 bits
        push %ax
        push %bx
        # check the branch version
        and $0xf0, %bx
        shr $4, %bx
        and $0xf0, %ax
        shr $4, %ax
        cmp %bx, %ax
        ja checkBootSignature.oldVersion
        jl checkBootSignature.newVersion
        cmp $0xA, %bx                               # check for alpha version
        je checkBootSignature.alpha
        cmp $0xB, %bx                               # check for beta version
        je checkBootSignature.beta
        cmp $0xC, %bx                               # check for release version
        je checkBootSignature.majVersion
        # return $CBS_NOT_VALID_SIGNATURE if the branch is not supported
        checkBootSignature.alpha:
            mov $CBS_ALPHA_SIGNATURE, %ax
            jmp checkBootSignature.end
        checkBootSignature.beta:
            mov $CBS_BETA_SIGNATURE, %ax
            jmp checkBootSignature.end
        # check the major version of the boot signature
        checkBootSignature.majVersion:
            # restore the original upper bits of the current signature and supplied signature
            pop %ax
            pop %bx
            # single out the major version number
            and $0xf, %bx
            and $0xf, %ax
            # compare the major version numbers
            cmp %bx, %ax
            je checkBootSignature.minVersion
            ja checkBootSignature.newVersion
            checkBootSignature.oldVersion:
                mov $CBS_OLD_SIGNATURE, %ax
                jmp checkBootSignature.end
            checkBootSignature.newVersion:
                mov $CBS_FUTURE_SIGNATURE, %ax
                jmp checkBootSignature.end
            checkBootSignature.minVersion:
                # get the lower half of the signature
                movw (CurrentBootSignature), %ax
                movw 4(%bp), %bx
                # single out the minor version number
                shr $8, %ax
                shr $8, %bx
                # compare the minor version numbers
                cmp %bx, %ax
                je checkBootSignature.validSignature
                ja checkBootSignature.oldVersion
                jmp checkBootSignature.newVersion
                checkBootSignature.validSignature:
                    mov $CBS_VALID_SIGNATURE, %ax
    checkBootSignature.end:
        mov %bp, %sp
        pop %bp
        ret

.section .data

.globl CurrentBootSignature
CurrentBootSignature: .long 0x23A1035C # 2023 - Alpha 1.02 - 5awCon
