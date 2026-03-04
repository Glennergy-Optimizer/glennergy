#!/bin/sh
set -e


if [ -f Makefile ]; then
    echo "Building Server"
    make
    sudo make install
fi

#crontab -l 2>/dev/null | grep -v "# Glennergy" | {
#    cat
#    echo "* * * * * /usr/local/bin/Glennergy-Spotpris # Glennergy-Spotpris cron"
#    echo "* * * * * /usr/local/bin/Glennergy-Meteo # Glennergy-Meteo cron"
#} | crontab -

find . -type f -name Makefile | while read mf; do
    dir=$(dirname "$mf")
    echo "Processing $dir"
    make -C "$dir"
    sudo make -C "$dir" install
done