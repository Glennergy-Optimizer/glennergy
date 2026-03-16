#!/bin/bash

set -e

echo "=========================================="
echo "  Glennergy Systemd Installation"
echo "  (Option 1: Server manages Cache+Algorithm)"
echo "=========================================="
echo

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script needs sudo privileges. Please run with sudo."
   exit 1
fi

# ============================================
# Step 1: Create System User
# ============================================
echo "[1/6] Setting up system user..."

# Ensure group exists
if ! getent group glennergy >/dev/null; then
    groupadd --system glennergy
    echo "      ✓ Created 'glennergy' group"
else
    echo "      ✓ Group 'glennergy' already exists"
fi

# Ensure user exists
if ! id -u glennergy >/dev/null 2>&1; then
    useradd -r -g glennergy -s /bin/false -d /var/lib/glennergy -c "Glennergy Service User" glennergy
    echo "      ✓ Created 'glennergy' user"
else
    echo "      ✓ User 'glennergy' already exists"
fi

# ============================================
# Step 2: Create Directories with Correct Permissions
# ============================================
echo "[2/6] Setting up directories..."

# Data directories
mkdir -p /var/lib/glennergy
mkdir -p /var/cache/glennergy/{meteo,spotpris}

# Log directory
mkdir -p /var/log/glennergy

# Runtime directory (for PID files, sockets, etc)
mkdir -p /run/glennergy

# Set ownership
chown -R glennergy:glennergy /var/lib/glennergy
chown -R glennergy:glennergy /var/cache/glennergy
chown -R glennergy:glennergy /var/log/glennergy
chown -R glennergy:glennergy /run/glennergy

# Set permissions
chmod 755 /var/lib/glennergy
chmod 755 /var/cache/glennergy
chmod 755 /var/log/glennergy
chmod 755 /run/glennergy

echo "      ✓ /var/lib/glennergy"
echo "      ✓ /var/cache/glennergy/{meteo,spotpris}"
echo "      ✓ /var/log/glennergy"
echo "      ✓ /run/glennergy"

# ============================================
# Step 3: Set Up IPC Resources
# ============================================
echo "[3/6] Setting up IPC resources..."

# Remove old FIFOs if they exist
rm -f /tmp/fifo_meteo /tmp/fifo_spotpris /tmp/fifo_notify_algorithm

# Create FIFOs
mkfifo /tmp/fifo_meteo
mkfifo /tmp/fifo_spotpris
mkfifo /tmp/fifo_notify_algorithm
# Set ownership and permissions
chown glennergy:glennergy /tmp/fifo_meteo /tmp/fifo_spotpris /tmp/fifo_notify_algorithm 2>/dev/null || true
chmod 0660 /tmp/fifo_meteo /tmp/fifo_spotpris /tmp/fifo_notify_algorithm 2>/dev/null || true

echo "      ✓ FIFOs created (/tmp/fifo_meteo, /tmp/fifo_spotpris, /tmp/fifo_notify_algorithm)"

# Socket will be created by the server at runtime
# Shared memory will be created by InputCache at runtime

# ============================================
# Step 4: Remove Old Cron Jobs
# ============================================
echo "[4/6] Cleaning up old cron jobs..."

if crontab -l 2>/dev/null | grep -q "Glennergy"; then
    crontab -l 2>/dev/null | grep -v "Glennergy" | crontab - 2>/dev/null || true
    echo "      ✓ Removed old cron jobs"
else
    echo "      ✓ No old cron jobs found"
fi

# ============================================
# Step 5: Install Systemd Services
# ============================================
echo "[5/6] Installing systemd services..."

if [ ! -d "systemd" ]; then
    echo "      ERROR: systemd/ directory not found!"
    echo "      Make sure service files are in systemd/ directory"
    exit 1
fi

# Copy service files
cp systemd/glennergy-server.service /etc/systemd/system/
cp systemd/glennergy-meteo.service /etc/systemd/system/
cp systemd/glennergy-meteo.timer /etc/systemd/system/
cp systemd/glennergy-spotpris.service /etc/systemd/system/
cp systemd/glennergy-spotpris.timer /etc/systemd/system/

# Reload systemd
systemctl daemon-reload

# Enable services
systemctl enable glennergy-server.service
systemctl enable glennergy-meteo.timer
systemctl enable glennergy-spotpris.timer

echo "      ✓ Systemd services installed and enabled"

# ============================================
# Step 6: Start Services
# ============================================
echo "[6/6] Starting services..."

# Stop any old instances
systemctl stop glennergy-server 2>/dev/null || true
systemctl stop glennergy-meteo.timer 2>/dev/null || true
systemctl stop glennergy-spotpris.timer 2>/dev/null || true
sleep 1

# Start the main server (which forks Cache and Algorithm)
echo "      Starting Glennergy Server (includes Cache + Algorithm)..."
systemctl start glennergy-server.service
sleep 3

if systemctl is-active --quiet glennergy-server; then
    echo "      ✓ Server running"
else
    echo "      ✗ Server failed to start!"
    echo ""
    echo "Error log:"
    journalctl -u glennergy-server -n 20 --no-pager
    exit 1
fi

# Start timers for periodic data fetching
echo "      Starting timers..."
systemctl start glennergy-meteo.timer
systemctl start glennergy-spotpris.timer
echo "      ✓ Timers started"

# Trigger initial data fetch
echo "      Fetching initial data..."
echo "        Triggering Meteo fetch..."
systemctl start glennergy-meteo.service || echo "        (Meteo fetch failed - will retry on next timer)"
sleep 2
echo "        Triggering Spotpris fetch..."
systemctl start glennergy-spotpris.service || echo "        (Spotpris fetch failed - will retry on next timer)"

echo
echo "=========================================="
echo "  Installation Complete!"
echo "=========================================="
echo
echo "Services Status:"
echo "----------------"
systemctl status glennergy-server --no-pager -l | head -4
echo ""
echo "Timers:"
echo "-------"
systemctl list-timers glennergy-* --no-pager
echo ""
echo "Useful commands:"
echo "  View server logs:    sudo journalctl -u glennergy-server -f"
echo "  View meteo logs:     sudo journalctl -u glennergy-meteo -f"
echo "  View spotpris logs:  sudo journalctl -u glennergy-spotpris -f"
echo "  Restart server:      sudo systemctl restart glennergy-server"
echo "  Check status:        sudo systemctl status glennergy-server"
echo "  Stop all:            sudo systemctl stop glennergy-server glennergy-meteo.timer glennergy-spotpris.timer"
echo ""
