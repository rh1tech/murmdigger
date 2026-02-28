#!/bin/bash
# flash.sh - Flash Script
#
# Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
# https://rh1.tech
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Default to ELF file from build directory
FIRMWARE="${1:-./build/murmdigger.elf}"

# Check if firmware file exists
if [ ! -f "$FIRMWARE" ]; then
    # Try .uf2 if .elf not found
    FIRMWARE="${FIRMWARE%.elf}.uf2"
    if [ ! -f "$FIRMWARE" ]; then
        echo "Error: Firmware file not found"
        echo "Usage: $0 [firmware.elf|firmware.uf2]"
        echo "Default: ./build/murmdigger.elf"
        exit 1
    fi
fi

echo "Flashing: $FIRMWARE"
picotool load -f "$FIRMWARE" && picotool reboot -f
