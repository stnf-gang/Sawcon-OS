# Documentation For The "SCIM" Image Manipulation Tool

### Written: Sunday 14th May 2023

### Last Updated: Thursday 18th May 2023

<br>

## Arguments & Switches

* -h / --help : Displays information about how to use the tool
* -m / --mode : Sets the mode of the tool
* -d / --disk : Selects the disk image to manipulate
* -i / --input : Selects the input file

## Modes

### read mode

In read mode, the tool will find the input file on the disk image and copy it into a file on the host machine.

### list mode

In list mode, the tool will find all of the files on the disk and print them onto the screen.

## Fatal Errors (Exxxx)

* E0001 - No Parameters
* E0002 - Invalid Argument Syntax
* E0003 - Invalid Mode
* E0004 - Invalid Argument
* E0005 - Could Not Open Disk Image
* E0006 - Disk Image Not Specified
* E0007 - Could Not Read Boot Sector Of Disk Image
* E0008 - Could Not Read Root Directory Of Disk Image
* E0009 - Input File Not Specified
* E000a - Could Not Open Input File
