# makefile
# 
# build scripts for SawconOS, its components, and any host tools being used
# This file was written for the SawconOS Host Tools
#
# Written: Monday 7th August 2023
# Last Updated: Tuesday 3rd August 2023
# 
# Written by Gabriel Jickells

# ===Directories===
# ==Output Directories==
BIN=bin
TMP=$(BIN)/tmp
SYSTEM_LIBS_BIN=$(BIN)/system
PROGLOAD_LIBS_BIN=$(SYSTEM_LIBS_BIN)/progload
# ==Source Code Directories==
SRC=src
BOOTLOADER_SRC=$(SRC)/bootloader
TOOLS_SRC=$(SRC)/tools
SCIM_SRC=$(TOOLS_SRC)/SCIM
SYSTEM_SRC=$(SRC)/system
PROGLOAD_SRC=$(SYSTEM_SRC)/progload

# ===Compilers & Tools===
TARGET_ASM=as
TARGET_LD=ld
HOST_CPP=g++
EMULATOR=qemu-system-i386

# ===Compiler Flags===
BOOTSECT_LDFLAGS=-Ttext 0x7c00 -e 0x7c00 --oformat binary
SCIM_CXXFLAGS=-Wall -Werror -Wpedantic
PROGLOAD_LDFLAGS=-T $(PROGLOAD_SRC)/progload.ld
PROGLOAD_ASMFLAGS=-I $(SYSTEM_SRC)
PROGLOAD_LIBS=$(SYSTEM_LIBS_BIN)/misc_functions.o $(PROGLOAD_LIBS_BIN)/signature.o $(SYSTEM_LIBS_BIN)/disk.o

# ===Disk Image Information===
VERSION=Alpha_1.0
DISK_NAME=SawconOS-Full-$(VERSION).img
SECT_COUNT=2880

# builds a disk image that contains SawconOS
disk: dirs bootloader system tools_SCIM
	dd if=/dev/zero of=$(BIN)/$(DISK_NAME) bs=512 count=$(SECT_COUNT)
	dd if=$(BIN)/SawconOS-Bootloader-boot_sector.bin of=$(BIN)/$(DISK_NAME) conv=notrunc
	bin/scim write -i $(BIN)/$(DISK_NAME) -f $(BIN)/progload.bin -e "SAWCON  BIN"

# compiles the SawconOS bootloader, including the boot sector and other files that are required to be in the final disk image
bootloader: dirs
	$(TARGET_ASM) $(BOOTLOADER_SRC)/boot.s -o $(TMP)/bootsect.o
	$(TARGET_LD) $(BOOTSECT_LDFLAGS) $(TMP)/bootsect.o -o $(BIN)/SawconOS-Bootloader-boot_sector.bin

# compiles the SawconOS System, including the SawconOS System Libs
system: dirs
	$(TARGET_ASM) $(PROGLOAD_ASMFLAGS) $(PROGLOAD_SRC)/progload.s -o $(TMP)/progload.o
	$(TARGET_ASM) $(SYSTEM_SRC)/misc_functions.s -o $(SYSTEM_LIBS_BIN)/misc_functions.o
	$(TARGET_ASM) $(PROGLOAD_SRC)/signature.s -o $(PROGLOAD_LIBS_BIN)/signature.o
	$(TARGET_ASM) $(SYSTEM_SRC)/disk.s -o $(SYSTEM_LIBS_BIN)/disk.o
	$(TARGET_LD) $(PROGLOAD_LDFLAGS) $(TMP)/progload.o $(PROGLOAD_LIBS) -o $(BIN)/progload.bin

# creates the compilation destination directories
dirs:
	mkdir -p $(BIN)
	mkdir -p $(TMP)
	mkdir -p $(SYSTEM_LIBS_BIN)
	mkdir -p $(PROGLOAD_LIBS_BIN)

# runs SawconOS in an emulator
run:
	$(EMULATOR) -drive if=floppy,format=raw,file=$(BIN)/$(DISK_NAME)

# compiles the SCIM host tool
tools_SCIM:
	$(HOST_CPP) $(SCIM_CXXFLAGS) $(SCIM_SRC)/scim.cpp -o $(BIN)/scim
