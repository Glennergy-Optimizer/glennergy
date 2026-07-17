#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
LIBEXEC_DIR=/usr/local/libexec/glennergy
UNIT_DIR=/etc/systemd/system
CONFIG_DIR=/etc/glennergy
CONFIG_FILE=$CONFIG_DIR/fastigheter.json
BACKUP_ROOT=/var/backups/glennergy
BACKUP_DIR=$BACKUP_ROOT/deploy-$(date -u +%Y%m%dT%H%M%SZ)

UNITS="
glennergy.target
glennergy-inputcache.service
glennergy-algorithm.service
glennergy-server.service
glennergy-meteo.service
glennergy-meteo.timer
glennergy-spotpris.service
glennergy-spotpris.timer
"

DEPLOY_STARTED=0
DEPLOY_SUCCEEDED=0
WAS_ACTIVE=0
WAS_ENABLED=0

fail()
{
    echo "Deployment failed: $*" >&2
    exit 1
}

require_command()
{
    command -v "$1" >/dev/null 2>&1 || fail "required command not found: $1"
}

restore_previous_installation()
{
    echo "Restoring the previous Glennergy installation..." >&2
    systemctl stop glennergy.target >/dev/null 2>&1 || true
    make -C "$SCRIPT_DIR" uninstall >/dev/null 2>&1 || true

    if [ -d "$BACKUP_DIR/libexec" ]; then
        install -d -m 0755 "$LIBEXEC_DIR"
        cp -a "$BACKUP_DIR/libexec/." "$LIBEXEC_DIR/"
    fi

    if [ -d "$BACKUP_DIR/units" ]; then
        for unit in "$BACKUP_DIR"/units/*; do
            [ -f "$unit" ] || continue
            cp -a "$unit" "$UNIT_DIR/"
        done
    fi

    systemctl daemon-reload >/dev/null 2>&1 || true
    if [ "$WAS_ENABLED" -eq 1 ]; then
        systemctl enable glennergy.target >/dev/null 2>&1 || true
    else
        systemctl disable glennergy.target >/dev/null 2>&1 || true
    fi
    if [ "$WAS_ACTIVE" -eq 1 ]; then
        systemctl start glennergy.target >/dev/null 2>&1 || true
    fi

    echo "Automatic rollback attempted. Backup retained at $BACKUP_DIR" >&2
}

on_exit()
{
    status=$?
    trap - EXIT HUP INT TERM

    if [ "$status" -ne 0 ] && [ "$DEPLOY_STARTED" -eq 1 ] && [ "$DEPLOY_SUCCEEDED" -eq 0 ]; then
        restore_previous_installation
    fi

    exit "$status"
}

trap on_exit EXIT
trap 'exit 130' HUP INT TERM

[ "$(id -u)" -eq 0 ] || fail "run this script with sudo"

for command_name in getent groupadd useradd install make systemctl systemd-analyze ss grep cp chown chmod date id awk flock sleep; do
    require_command "$command_name"
done

exec 9>/run/lock/glennergy-deploy.lock
flock -n 9 || fail "another Glennergy deployment is already running"

make -C "$SCRIPT_DIR" check-runtime-dependencies

if ! getent group glennergy >/dev/null 2>&1; then
    groupadd --system glennergy
fi

if ! getent passwd glennergy >/dev/null 2>&1; then
    useradd --system \
        --gid glennergy \
        --home-dir /var/lib/glennergy \
        --no-create-home \
        --shell /usr/sbin/nologin \
        glennergy
elif [ "$(id -gn glennergy)" != "glennergy" ]; then
    fail "existing glennergy user does not use the glennergy primary group"
elif [ "$(getent passwd glennergy | awk -F: '{print $7}')" != "/usr/sbin/nologin" ]; then
    fail "existing glennergy user does not use /usr/sbin/nologin"
fi

install -d -m 0700 "$BACKUP_ROOT"
install -d -m 0700 "$BACKUP_DIR"
install -d -m 0700 "$BACKUP_DIR/units"

if systemctl is-active --quiet glennergy.target; then
    WAS_ACTIVE=1
fi
if systemctl is-enabled --quiet glennergy.target; then
    WAS_ENABLED=1
fi

if [ -d "$LIBEXEC_DIR" ]; then
    cp -a "$LIBEXEC_DIR" "$BACKUP_DIR/libexec"
fi

for unit in $UNITS; do
    if [ -f "$UNIT_DIR/$unit" ]; then
        cp -a "$UNIT_DIR/$unit" "$BACKUP_DIR/units/"
    fi
done

if [ -f "$CONFIG_FILE" ]; then
    cp -a "$CONFIG_FILE" "$BACKUP_DIR/fastigheter.json"
fi

DEPLOY_STARTED=1
if systemctl cat glennergy.target >/dev/null 2>&1; then
    systemctl stop glennergy.target
fi

install -d -o root -g glennergy -m 0750 "$CONFIG_DIR"

make -C "$SCRIPT_DIR" install

chown root:glennergy "$CONFIG_FILE"
chmod 0640 "$CONFIG_FILE"
install -d -o glennergy -g glennergy -m 0750 /var/lib/glennergy

systemctl daemon-reload
systemd-analyze verify \
    "$UNIT_DIR/glennergy.target" \
    "$UNIT_DIR/glennergy-inputcache.service" \
    "$UNIT_DIR/glennergy-algorithm.service" \
    "$UNIT_DIR/glennergy-server.service" \
    "$UNIT_DIR/glennergy-meteo.service" \
    "$UNIT_DIR/glennergy-meteo.timer" \
    "$UNIT_DIR/glennergy-spotpris.service" \
    "$UNIT_DIR/glennergy-spotpris.timer"

systemctl enable --now glennergy.target

for active_unit in \
    glennergy-inputcache.service \
    glennergy-algorithm.service \
    glennergy-server.service \
    glennergy-meteo.timer \
    glennergy-spotpris.timer; do
    systemctl is-active --quiet "$active_unit" || fail "unit is not active: $active_unit"
done

http_ready=0
attempt=1
while [ "$attempt" -le 30 ]; do
    if ss -H -ltn 'sport = :8080' | grep -q '127.0.0.1:8080'; then
        http_ready=1
        break
    fi
    sleep 1
    attempt=$((attempt + 1))
done

if [ "$http_ready" -ne 1 ]; then
    fail "Glennergy is not listening on 127.0.0.1:8080"
fi

for active_unit in \
    glennergy-inputcache.service \
    glennergy-algorithm.service \
    glennergy-server.service \
    glennergy-meteo.timer \
    glennergy-spotpris.timer; do
    systemctl is-active --quiet "$active_unit" || fail "unit became inactive during health checks: $active_unit"
done

DEPLOY_SUCCEEDED=1
echo "Glennergy deployment succeeded. Backup retained at $BACKUP_DIR"
