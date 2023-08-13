# Changelog

## SCIM - Alpha 1.0 (Saturday 12th August 2023 - Sunday 13th August 2023)
### Changes
#### Saturday 12th August 2023
- the build system is now set up to compile SCIM
#### Sunday 13th August 2023
- added functions and data structures to parse the boot record of FAT12 formatted disks
- added data structures to parse the root directory of FAT12 formatted disks
### Issues
none found in testing

## SawconOS Bootloader - BIOS disk read test (Friday 11th August 2023)

### Changes
- The SawconOS bootloader will now read some data from the second sector of the boot disk and print it to the screen
- added puts function for printing strings to the screen
- added a script in the makefile to build a complete floppy disk image. This should prevent unexpected read errors in emulators if SawconOS tries to read sectors that exist outside of the boot sector code
### Issues
- no disk error handling, which is a problem for floppy disks because they are very unreliable. This doesn't cause any significant issues yet but it very well could when it comes to reading actual code from the disk
### Fixes
- the video mode is now set to a known value in the boot sector so there are no issues with unexpected video modes

## SawconOS Bootloader - BIOS function test (Thursday 10th August 2023)

### Changes
- the SawconOS bootloader now prints a character to the screen using one of the BIOS video functions
- the CPU will still be halted after the boot sector code concludes as to prevent possible problems with random memory data being misconstrued as code
### Issues
- The function to write a character to the screen may act unexpectedly if the BIOS loads the computer in a graphical mode by default, instead of a text mode

## SawconOS Bootloader - BIOS boot test (Monday 7th August 2023)

### Changes
- the SawconOS bootloader can now be loaded using BIOS
- it will immediately halt the CPU upon being loaded
### Issues
none found in testing
