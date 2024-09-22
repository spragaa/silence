#!/bin/bash

DEBUG_MODE=false
if [[ "$1" == "--debug" ]]; then
    DEBUG_MODE=true
    shift
fi

debug_echo() {
    if [ "$DEBUG_MODE" = true ]; then
        local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
        echo "[DEBUG] [$timestamp] $1"
    fi
}

if [[ "$1" == "--clean" ]]; then
    debug_echo "rm -rf ../build/* ..."
    rm -rf ../build/*
fi

mkdir -p ../build

cd ../build || exit

cmake ..
debug_echo "'cmake ..' finished"

make -j$(nproc)
debug_echo "'make' finished"
