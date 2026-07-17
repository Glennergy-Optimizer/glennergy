#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
MODE=${1:-uninstall}
GLENNERGY_UNITS="
glennergy.target
glennergy-meteo.timer
glennergy-spotpris.timer
glennergy-server.service
glennergy-algorithm.service
glennergy-inputcache.service
glennergy-meteo.service
glennergy-spotpris.service
"

stop_installed_units()
{
    for unit in $GLENNERGY_UNITS; do
        if systemctl cat "$unit" >/dev/null 2>&1; then
            systemctl stop "$unit"
        fi
    done

    if systemctl cat glennergy.target >/dev/null 2>&1; then
        systemctl disable glennergy.target
    fi
}

reset_glennergy_failures()
{
    systemctl reset-failed $GLENNERGY_UNITS >/dev/null 2>&1 || true
}

if [ "$(id -u)" -ne 0 ]; then
    echo "Removal writes to system directories." >&2
    echo "Run with sudo." >&2
    exit 1
fi

case "$MODE" in
    uninstall)
        stop_installed_units
        make -C "$SCRIPT_DIR" uninstall
        systemctl daemon-reload
        reset_glennergy_failures
        echo "Glennergy programs removed; configuration and data were preserved."
        ;;
    purge)
        if [ "${2:-}" != "--confirm" ]; then
            echo "Purge permanently removes Glennergy configuration, state, and cache." >&2
            echo "Run: sudo /bin/sh ./glennergy_uninstall.sh purge --confirm" >&2
            exit 2
        fi
        stop_installed_units
        make -C "$SCRIPT_DIR" CONFIRM_PURGE=YES purge
        systemctl daemon-reload
        reset_glennergy_failures

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
