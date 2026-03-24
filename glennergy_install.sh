#!/bin/sh
set -e


if [ -f Makefile ]; then
    echo "Building Server"
    make
    sudo make install
fi

find . -type f -name Makefile | while read mf; do
    dir=$(dirname "$mf")
    echo "Processing $dir"
    make -C "$dir"
    sudo make -C "$dir" install
done

APP_GROUP="glennergy"

echo "Setting up glennergy system directories..."

# Create group if missing
if ! getent group "$APP_GROUP" >/dev/null; then
  sudo groupadd "$APP_GROUP"
fi

# Add current user to group
sudo usermod -aG "$APP_GROUP" "$USER"

# Create directories
sudo mkdir -p /var/log/glennergy
sudo mkdir -p /var/cache/glennergy/meteo
sudo mkdir -p /var/cache/glennergy/spotpris

# Set ownership and permissions
sudo chown root:"$APP_GROUP" /var/log/glennergy /var/cache/glennergy/meteo /var/cache/glennergy/spotpris
sudo chmod 2775 /var/log/glennergy /var/cache/glennergy/meteo /var/cache/glennergy/spotpris

echo "Installation complete."
echo "You must activate the new group before running the program:"
echo
echo "    newgrp $APP_GROUP"
echo
echo "or log out and back in."