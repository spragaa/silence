#!/bin/bash

debug_echo() {
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    echo "[DEBUG] [$timestamp] $1"
}

if [[ "$1" == "--clean" ]]; then
    debug_echo "rm -rf ../build/server ..."
    rm -rf ../build/server
fi

mkdir -p ../build || exit

cd ../build || exit

cmake -DBUILD_TESTS=ON -DDEBUG=ON ..
debug_echo "'cmake ..' finished"

make -j$(nproc)
debug_echo "'make tests' finished"

../build/tests/test_server/test_server 