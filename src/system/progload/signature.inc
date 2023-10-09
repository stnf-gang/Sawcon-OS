# misc_functions.inc
#
# includes all the external references for signature.o
# the program still needs to link with signature.o
# Written for use with assembly files
# Written for SawconOS System Alpha 1.0
#
# Written: Friday 6th October 2023
# Last Updated: Saturday 7th October 2023
#
# Written by Gabriel Jickells

.extern checkBootSignature                  # void __cdecl checkBootSignature(unsigned long Signature)
.equ CBS_NOT_5C_SIGNATURE, 3
.equ CBS_ALPHA_SIGNATURE, 4
.equ CBS_BETA_SIGNATURE, 5
.equ CBS_NOT_VALID_SIGNATURE, 3
.equ CBS_FUTURE_SIGNATURE, 2
.equ CBS_OLD_SIGNATURE, 1
.equ CBS_VALID_SIGNATURE, 0
