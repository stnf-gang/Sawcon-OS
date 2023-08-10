# Changelog

## SawconOS Bootloader - BIOS function test (Thursday 10th August 2023)
- the SawconOS bootloader now prints a character to the screen using one of the BIOS video functions
- the CPU will still be halted after the boot sector code concludes as to prevent possible problems with random memory data being misconstrued as code

## SawconOS Bootloader - BIOS boot test (Monday 7th August 2023)
- the SawconOS bootloader can now be loaded using BIOS
- it will immediately halt the CPU upon being loaded
