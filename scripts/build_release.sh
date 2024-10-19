#!/bin/bash

DEBUG_MODE=false
CLEAN_MODE=false

while [[ $# -gt 0 ]]; do
    case "$1" in
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

if [ "$CLEAN_MODE" = true ]; then
    echo "Cleaning build directory..."
    rm -rf ../build/*
fi

mkdir -p ../build
cd ../build || exit

cmake -DDEBUG=OFF -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release ..

make -j$(nproc)
