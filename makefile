# makefile
# 
# build script for SawconOS and its components
# This file was written for the SawconOS Host Tools
#
# Written: Monday 7th August 2023
# Last Updated: Monday 7th August 2023
# 
# Written by Gabriel Jickells

# ===Directories===
# ==Output Directories==
BIN=bin
TMP=$(BIN)/tmp
# ==Source Code Directories==
SRC=src
BOOTLOADER_SRC=$(SRC)/bootloader

# ===Compilers===
TARGET_ASM=as
TARGET_LD=ld

# ===Compiler Flags===
BOOTSECT_LDFLAGS=-Ttext 0x7c00 -e 0x7c00 --oformat binary

bootloader: dirs
	$(TARGET_ASM) $(BOOTLOADER_SRC)/boot.s -o $(TMP)/bootsect.o
	$(TARGET_LD) $(BOOTSECT_LDFLAGS) $(TMP)/bootsect.o -o $(BIN)/SawconOS-Bootloader-boot_sector.bin

dirs:
	mkdir -p $(BIN)
	mkdir -p $(TMP)
