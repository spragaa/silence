#!/bin/bash

DEBUG_MODE=false
NICKNAME="noname"
PASSWORD=""
EXTRA_ARGS=""

while [[ $# -gt 0 ]]; do
    case "\$1" in
        --debug)
            DEBUG_MODE=true
            shift
            ;;
        --nickname=*)
            NICKNAME="${1#*=}"
            shift
            ;;
        --nickname)
            NICKNAME="\$2"
            shift 2
            ;;
        --password=*)
            PASSWORD="${1#*=}"
            shift
            ;;
        --password)
            PASSWORD="\$2"
            shift 2
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

CLIENT_ARGS="--nickname=\"$NICKNAME\""
if [ -n "$PASSWORD" ]; then
    CLIENT_ARGS="$CLIENT_ARGS --password=\"$PASSWORD\""
fi

CLIENT_ARGS="$CLIENT_ARGS $EXTRA_ARGS"

debug_echo "Executing client with arguments: $CLIENT_ARGS"
eval "../build/client/client $CLIENT_ARGS"