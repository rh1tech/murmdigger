#!/bin/bash
# Build murmdigger - Digger Remastered for RP2350
#
# Usage: ./build.sh [OPTIONS]
#   -b, --board      Board variant: M1 (default) or M2
#   -c, --cpu        CPU speed in MHz: 252 (default), 378, 504
#   -h, --help       Show this help

# Defaults
BOARD="M2"
CPU="252"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -b|--board)
            BOARD="$2"
            shift 2
            ;;
        -c|--cpu)
            CPU="$2"
            shift 2
            ;;
        -h|--help)
            head -8 "$0" | tail -6
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Build cmake arguments
CMAKE_ARGS="-DPICO_BOARD=pico2"
CMAKE_ARGS="$CMAKE_ARGS -DBOARD_VARIANT=$BOARD"
CMAKE_ARGS="$CMAKE_ARGS -DCPU_SPEED=$CPU"

echo "Building murmdigger:"
echo "  Board: $BOARD"
echo "  CPU: $CPU MHz"
echo ""

rm -rf ./build
mkdir build
cd build
cmake $CMAKE_ARGS ..
make -j4
