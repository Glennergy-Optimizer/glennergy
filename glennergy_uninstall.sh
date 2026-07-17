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
        systemctl disable --now glennergy.target >/dev/null 2>&1 || true
        make -C "$SCRIPT_DIR" uninstall
        systemctl daemon-reload
        systemctl reset-failed >/dev/null 2>&1 || true
        echo "Glennergy programs removed; configuration and data were preserved."
        ;;
    purge)
        if [ "${2:-}" != "--confirm" ]; then
            echo "Purge permanently removes Glennergy configuration, state, and cache." >&2
            echo "Run: sudo /bin/sh ./glennergy_uninstall.sh purge --confirm" >&2
            exit 2
        fi
        systemctl disable --now glennergy.target >/dev/null 2>&1 || true
        make -C "$SCRIPT_DIR" CONFIRM_PURGE=YES purge
        systemctl daemon-reload
        systemctl reset-failed >/dev/null 2>&1 || true

        if getent passwd glennergy >/dev/null 2>&1; then
            userdel glennergy
        fi

        if getent group glennergy >/dev/null 2>&1; then
            group_gid=$(getent group glennergy | awk -F: '{print $3}')
            supplementary_members=$(getent group glennergy | awk -F: '{print $4}')
            primary_members=$(getent passwd | awk -F: -v gid="$group_gid" '$4 == gid {print $1}' | awk 'BEGIN { ORS=" " } { print }')

            if [ -z "$supplementary_members" ] && [ -z "$primary_members" ]; then
                groupdel glennergy
            else
                echo "Preserved glennergy group because it is still used by: ${supplementary_members:-$primary_members}"
            fi
        fi
        ;;
    *)
        echo "Usage: sudo /bin/sh ./glennergy_uninstall.sh [uninstall|purge --confirm]" >&2
        exit 2
        ;;
esac
