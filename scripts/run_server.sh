#!/bin/bash

DEBUG_MODE=false
EXTRA_ARGS=""

while [[ $# -gt 0 ]]; do
    case "\$1" in
        --debug)
            DEBUG_MODE=true
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

SERVER_ARGS="$EXTRA_ARGS"

debug_echo "Executing server with arguments: $SERVER_ARGS"
eval "../build/server/server $SERVER_ARGS"