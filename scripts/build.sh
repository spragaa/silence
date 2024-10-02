#!/bin/bash

DEBUG_MODE=false
CLEAN_MODE=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug)
            DEBUG_MODE=true
            shift
            ;;
        --clean)
            CLEAN_MODE=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

debug_echo() {
    if [ "$DEBUG_MODE" = true ]; then
        local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
        echo "[DEBUG] [$timestamp] $1"
    fi
}

if [ "$CLEAN_MODE" = true ]; then
    debug_echo "Cleaning build directory..."
    rm -rf ../build/*
fi

mkdir -p ../build
cd ../build || exit

if [ "$DEBUG_MODE" = true ]; then
    debug_echo "Running CMake with DEBUG=ON"
    cmake -DDEBUG=ON ..
else
    debug_echo "Running CMake with DEBUG=OFF"
    cmake -DDEBUG=OFF ..
fi

debug_echo "'cmake ..' finished"

make -j$(nproc)
debug_echo "'make -j$(nproc)' finished"
