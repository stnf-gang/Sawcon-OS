# makefile
# 
# build script for SawconOS and its components
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

# ===Compilers & Tools===
TARGET_ASM=as
TARGET_LD=ld
EMULATOR=qemu-system-i386

# ===Compiler Flags===
BOOTSECT_LDFLAGS=-Ttext 0x7c00 -e 0x7c00 --oformat binary

# ===Disk Image Information===
VERSION=BIOS_READ_TEST
DISK_NAME=SawconOS-Full-$(VERSION).img
SECT_COUNT=2880

disk: dirs bootloader
	dd if=/dev/zero of=$(BIN)/$(DISK_NAME) bs=512 count=$(SECT_COUNT)
	dd if=$(BIN)/SawconOS-Bootloader-boot_sector.bin of=$(BIN)/$(DISK_NAME) conv=notrunc

bootloader: dirs
	$(TARGET_ASM) $(BOOTLOADER_SRC)/boot.s -o $(TMP)/bootsect.o
	$(TARGET_LD) $(BOOTSECT_LDFLAGS) $(TMP)/bootsect.o -o $(BIN)/SawconOS-Bootloader-boot_sector.bin

dirs:
	mkdir -p $(BIN)
	mkdir -p $(TMP)

run:
	$(EMULATOR) -drive if=floppy,format=raw,file=$(BIN)/$(DISK_NAME)
