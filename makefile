# makefile
# 
# build scripts for SawconOS, its components, and any host tools being used
# This file was written for the SawconOS Host Tools
#
# Written: Monday 7th August 2023
# Last Updated: Friday 11th August 2023
# 
# Written by Gabriel Jickells

# ===Directories===
# ==Output Directories==
BIN=bin
TMP=$(BIN)/tmp
# ==Source Code Directories==
SRC=src
BOOTLOADER_SRC=$(SRC)/bootloader
TOOLS_SRC=$(SRC)/tools
SCIM_SRC=$(TOOLS_SRC)/SCIM

# ===Compilers & Tools===
TARGET_ASM=as
TARGET_LD=ld
HOST_CPP=g++
EMULATOR=qemu-system-i386

# ===Compiler Flags===
BOOTSECT_LDFLAGS=-Ttext 0x7c00 -e 0x7c00 --oformat binary
SCIM_CXXFLAGS=-Wall -Werror -Wpedantic

# ===Disk Image Information===
VERSION=BIOS_READ_TEST
DISK_NAME=SawconOS-Full-$(VERSION).img
SECT_COUNT=2880

# builds a disk image that contains SawconOS
disk: dirs bootloader
	dd if=/dev/zero of=$(BIN)/$(DISK_NAME) bs=512 count=$(SECT_COUNT)
	dd if=$(BIN)/SawconOS-Bootloader-boot_sector.bin of=$(BIN)/$(DISK_NAME) conv=notrunc

# compiles the SawconOS bootloader, including the boot sector and other files that are required to be in the final disk image
bootloader: dirs
	$(TARGET_ASM) $(BOOTLOADER_SRC)/boot.s -o $(TMP)/bootsect.o
	$(TARGET_LD) $(BOOTSECT_LDFLAGS) $(TMP)/bootsect.o -o $(BIN)/SawconOS-Bootloader-boot_sector.bin

# creates the compilation destination directories
dirs:
	mkdir -p $(BIN)
	mkdir -p $(TMP)

# runs SawconOS in an emulator
run:
	$(EMULATOR) -drive if=floppy,format=raw,file=$(BIN)/$(DISK_NAME)

# compiles the SCIM host tool
tools_SCIM:
	$(HOST_CPP) $(SCIM_CXXFLAGS) $(SCIM_SRC)/scim.cpp -o $(BIN)/scim
