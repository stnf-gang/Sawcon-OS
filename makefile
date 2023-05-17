# makefile
# 
# build script for SawconOS
# This file was written for SawconOS
#
# Written: Friday 12th May 2023
# Last Updated: Sunday 14th May 2023
#
# Written by Gabriel Jickells

# ===Directories===

SRC=src
BOOTLOADER=$(SRC)/bootloader
TOOLS=$(SRC)/tools

BIN=bin
TMP=$(BIN)/tmp

# ===Compilers===

TARGET_ASM=as
TARGET_LD=ld
HOST_CC=gcc

# ===Compiler Flags===

BOOTSECT_LDFLAGS=-Ttext 0x7c00 -e 0x7c00 --oformat binary

# ===Rules===

# builds SawconOS
all: bootloader

bootloader: dirs
	as $(BOOTLOADER)/boot.s -o $(TMP)/boot.o
	ld $(TMP)/boot.o $(BOOTSECT_LDFLAGS) -o $(TMP)/boot.bin

run:
	qemu-system-i386 -fda $(TMPDIR)/boot.bin

dirs:
	mkdir -p $(BIN)
	mkdir -p $(TMP)

tools_FAT12: dirs
	$(HOST_CC) $(TOOLS)/FAT12/fat12.c -o $(BIN)/scim
