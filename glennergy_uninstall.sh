#!/bin/sh
set -e


if [ -f Makefile ]; then
    echo "Uninstalling"
    sudo make uninstall
fi

find . -type f -name Makefile | while read mf; do
    dir=$(dirname "$mf")
    echo "Processing $dir"
    sudo make -C "$dir" uninstall

done