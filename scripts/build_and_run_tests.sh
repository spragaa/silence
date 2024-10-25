#!/bin/bash

DEBUG_MODE=false
CLEAN_MODE=false
EXTRA_ARGS=""

while [[ $# -gt 0 ]]; do
    case "\$1" in
        --debug)
            DEBUG_MODE=true
            shift
            ;;
        --clean)
            CLEAN_MODE=true
            shift
            ;;
        *)
            EXTRA_ARGS="$EXTRA_ARGS \$1"
            shift
            ;;
    esac
done

debug_echo() {
    if [ "$DEBUG_MODE" = true ]; then
        local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
        echo "[DEBUG] [$timestamp] \$1"
    fi
}

if [ "$CLEAN_MODE" = true ]; then
    debug_echo "Cleaning build directory..."
    rm -rf ../build/server
fi

mkdir -p ../build || exit
cd ../build || exit

if [ "$DEBUG_MODE" = true ]; then
    debug_echo "Running CMake with DDEBUG=ON"
    cmake -DBUILD_TESTS=ON -DDEBUG=ON ..
else
    debug_echo "Running CMake with DDEBUG=OFF"
    cmake -DDEBUG=OFF ..
fi
debug_echo "'cmake ..' finished"

make -j$(nproc)
debug_echo "'make' finished"

debug_echo "Executing tests with arguments: $EXTRA_ARGS"
eval "../build/tests/test_server/test_server $EXTRA_ARGS"