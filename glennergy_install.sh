#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

if [ "$(id -u)" -ne 0 ]; then
    echo "Installation writes to system directories." >&2
    echo "Build and validate first, then run: sudo ./glennergy_install.sh" >&2
    exit 1
fi

echo "Installing prebuilt Glennergy artifacts..."
make -C "$SCRIPT_DIR" install

echo "Static files installed."
echo "Service-account creation and systemd activation are handled by the deployment workflow."
