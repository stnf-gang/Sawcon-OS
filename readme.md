# Changelog

## SawconOS Bootloader - Alpha 1.03 (Monday 9th October 2023)
### Changes
#### Monday 9th September 2023
- Changed signature to 23A1035C
### Fixes
- Fixed a bug where the drive number was not set before reading a cluster

## SCIM - Alpha 1.4 (Saturday 7th October 2023)
### Changes
#### Saturday 7th October 2023
- added serialise/serialize mode for changing the serial number of the drive
- added -s/--serial switch for specifying what to change the serial number to

## SawconOS System - Alpha 1.0 (Tuesday 3rd October 2023 - Monday 9th October 2023)
- added PROGLOAD which can load any file from the root directory of the boot disk
- made some crude system libraries that programs can link with

## SawconOS Bootloader - Alpha 1.02 (Tuesday 3rd October 2023)
### Changes
#### Tuesday 3rd October 2023
- SawconOS Bootloader now passes the drive signature to the SawconOS System
- Changed signature to 23A1025C

## SawconOS Bootloader - Alpha 1.01 (Saturday 23rd September 2023)
### Changes
#### Saturday 23rd September 2023
- fixed a typo in the comments
- SawconOS Bootloader now resets the segment registers before passing control to the SawconOS System
- SawconOS Bootloader now passes the drive number to the SawconOS System
### Issues
- Drive signature should have changed to 23A1015C but didn't

## SawconOS Bootloader - Alpha 1.0 (Sunday 17th September 2023 - Wednesday 20th September 2023)
### Changes
#### Sunday 17th September 2023
- removed all test code
- added the FAT12 header
#### Monday 18th September 2023
- SawconOS Bootloader now gets drive geometry from the BIOS
- SawconOS Bootloader now sets the video mode to VGA text mode 80x25 16 colours
#### Tuesday 19th September 2023
- Added LBA2CHS function
- Added ReadSectors function
#### Wednesday 20th September 2023
- added ReadRootDirectory function
- added FindEntry function
- added ReadFAT function
- added ReadEntry function

## SCIM - Alpha 1.3 (Sunday 27th August 2023 - Sunday 17th September 2023)

### Changes
#### 27th August 2023
- rewrote scim.cpp from scratch
- added support for different operating modes inside of SCIM
- added -i/--image switch for specifying the disk image to work with
#### 30th August 2023
- renamed scim::FAT12::Disk.FindFile() to scim::FAT12::Disk.FindEntry()
- renamed scim::FAT12::Disk.ReadFile() to scim::FAT12::Disk.ReadEntry()
- added list mode which outputs a list of all entries present in the root directory
- added -e/--entry for specifying the target entry to work with
- added read mode which prints the contents of a file entry to stdout
- added delete mode which deletes a file entry
- added scim::FAT12::Disk.DeleteEntry() function
#### 31st August 2023
- added scim::FAT12::Disk.WriteEntry() function
- added scim::FAT12::Disk.WriteFAT() function
- added scim::FAT::NAME_SPECIAL enum
- added special character handling to list mode
- added write mode which copies a file from the host disk to a disk image file
#### 10th September 2023
- added scim::FAT::FILE_ATTRIBUTES enum
#### 11th - 17th September 2023
- finished write mode

### Issues
#### new
- no check in any of the FAT functions that Image is not a NULL pointer. Can't be exploited in SCIM but can be if fat.hpp is used in another program. Don't currently have plans of fixing it just get good lmao
#### from previous versions
- There is no check to make sure the values in the FAT header are valid which can lead to unexpected behavious in functions that require them

### Fixes
#### From SCIM Alpha 1.2
##### 30th August 2023
- SCIM is now memory safe

## SCIM - Alpha 1.2 (Wednesday 16th August 2023)

### Changes
- added scim::FAT12::Disk.ReadFile() for reading the data of a file from its meta-data
- added scim::FAT12::Disk.Cluster2LBA() for converting clusters to LBA values when they need to be read
- scim::FAT12::Disk.ReadRootDirectory() now fills the value in DataSectionLBA and as so must be called before any functions that need them such as ReadFile or Cluster2LBA
- SCIM now reads a file, specified by -f, from a disk image and prints its contents to the screen

### Issues
#### new
- SCIM isn't memory safe
#### from previous versions
- There is no check to make sure the values in the FAT header are valid which can lead to unexpected behavious in functions that require them

### Fixes
- scim::FAT12::Disk.FindFile() now checks that the root directory has been read

## SCIM - Alpha 1.1 (Tuesday 15th August 2023)

### Changes
- added functions for reading the root directory of FAT12 formatted disks
- added scim::FAT12::Disk.FindFile() for locating file metadata in the root directory of FAT12 formatted disks

### Issues
#### new
- scim::FAT12::Disk.FindFile() has no check to make sure the root directory has been read
#### from previous versions
- There is no check to make sure the values in the FAT header are valid which can lead to unexpected behaviors in functions that require them

### Fixes
- SCIM now checks that a disk image is specified before trying to read it which fixes a strangely formatted error message issue when SCIM tries to read the image

## SCIM - Alpha 1.0 (Saturday 12th August 2023 - Sunday 13th August 2023)

### Changes
#### Saturday 12th August 2023
- the build system is now set up to compile SCIM
#### Sunday 13th August 2023
- added functions and data structures to parse the boot record of FAT12 formatted disks
- added data structures to parse the root directory of FAT12 formatted disks

### Issues
- Some weird text formatting issues when a disk image is not specified
- There is no check to make sure the values in the FAT header are valid which can lead to unexpected behaviors in functions that require them

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
none found
