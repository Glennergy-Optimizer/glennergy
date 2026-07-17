#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
MODE=${1:-uninstall}

if [ "$(id -u)" -ne 0 ]; then
    echo "Removal writes to system directories." >&2
    echo "Run with sudo." >&2
    exit 1
fi

case "$MODE" in
    uninstall)
        make -C "$SCRIPT_DIR" uninstall
        echo "Glennergy programs removed; configuration and data were preserved."
        ;;
    purge)
        if [ "${2:-}" != "--confirm" ]; then
            echo "Purge permanently removes Glennergy configuration, state, and cache." >&2
            echo "Run: sudo ./glennergy_uninstall.sh purge --confirm" >&2
            exit 2
        fi
        make -C "$SCRIPT_DIR" CONFIRM_PURGE=YES purge
        ;;
    *)
        echo "Usage: sudo ./glennergy_uninstall.sh [uninstall|purge --confirm]" >&2
        exit 2
        ;;
esac
