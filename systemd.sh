#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

sudo cp "$SCRIPT_DIR"/*.service "$SCRIPT_DIR"/*.timer /etc/systemd/system/

sudo systemctl daemon-reload

sudo systemctl start --now glennergy-main.service
sudo systemctl start --now glennergy-algoritm.timer
sudo systemctl start --now glennergy-meteo.timer
sudo systemctl start --now glennergy-spotpris.timer

echo "Services are a GOOOOOOO."