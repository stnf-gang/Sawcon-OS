# misc_functions.inc
#
# includes all the external references for misc_functions.o
# the program still needs to link with misc_functions.o
# Written for use with assembly files
# Written for SawconOS System Alpha 1.0
#
# Written: Tuesday 3rd October 2023
# Last Updated: Monday 9th October 2023
#
# Written by Gabriel Jickells

.extern putc                        # void __cdecl putc(char __c)
.extern puts                        # void __cdecl puts(char *__s)
.extern puts_len                    # void __cdecl puts_len(char *__s, int _length)
