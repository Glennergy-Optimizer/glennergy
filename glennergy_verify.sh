#!/bin/sh
set -u

UNIT_DIR=/etc/systemd/system
CONFIG_FILE=/etc/glennergy/fastigheter.json
FAILED=0

pass()
{
    echo "PASS: $*"
}

fail()
{
    echo "FAIL: $*" >&2
    FAILED=1
}

check_command()
{
    if command -v "$1" >/dev/null 2>&1; then
        pass "command available: $1"
    else
        fail "required command unavailable: $1"
    fi
}

for command_name in getent systemctl systemd-analyze stat ss grep curl id awk; do
    check_command "$command_name"
done

if [ "$FAILED" -ne 0 ]; then
    exit 1
fi

if getent passwd glennergy >/dev/null 2>&1; then
    service_group=$(id -gn glennergy 2>/dev/null || true)
    service_shell=$(getent passwd glennergy | awk -F: '{print $7}')
    [ "$service_group" = "glennergy" ] && pass "service account uses glennergy as its primary group" || fail "service account has unexpected primary group: $service_group"
    [ "$service_shell" = "/usr/sbin/nologin" ] && pass "service account is non-login" || fail "service account has unexpected shell: $service_shell"
else
    fail "glennergy service account does not exist"
fi

if [ -r "$CONFIG_FILE" ]; then
    config_owner=$(stat -c '%U:%G' "$CONFIG_FILE")
    config_mode=$(stat -c '%a' "$CONFIG_FILE")
    [ "$config_owner" = "root:glennergy" ] && pass "configuration ownership is root:glennergy" || fail "configuration ownership is $config_owner"
    [ "$config_mode" = "640" ] && pass "configuration mode is 0640" || fail "configuration mode is $config_mode"
else
    fail "configuration is not readable: $CONFIG_FILE"
fi

if systemd-analyze verify \
    "$UNIT_DIR/glennergy.target" \
    "$UNIT_DIR/glennergy-inputcache.service" \
    "$UNIT_DIR/glennergy-algorithm.service" \
    "$UNIT_DIR/glennergy-server.service" \
    "$UNIT_DIR/glennergy-meteo.service" \
    "$UNIT_DIR/glennergy-meteo.timer" \
    "$UNIT_DIR/glennergy-spotpris.service" \
    "$UNIT_DIR/glennergy-spotpris.timer"; then
    pass "installed systemd units passed verification"
else
    fail "installed systemd units failed verification"
fi

if systemctl is-enabled --quiet glennergy.target; then
    pass "glennergy.target is enabled for boot"
else
    fail "glennergy.target is not enabled for boot"
fi

for unit in \
    glennergy.target \
    glennergy-inputcache.service \
    glennergy-algorithm.service \
    glennergy-server.service \
    glennergy-meteo.timer \
    glennergy-spotpris.timer; do
    if systemctl is-active --quiet "$unit"; then
        pass "unit is active: $unit"
    else
        fail "unit is not active: $unit"
    fi
done

listeners=$(ss -H -ltn 'sport = :8080')
if echo "$listeners" | grep -q '127.0.0.1:8080'; then
    pass "server listens on 127.0.0.1:8080"
else
    fail "server does not listen on 127.0.0.1:8080"
fi

if echo "$listeners" | grep -Eq '(^|[[:space:]])(0\.0\.0\.0|\[::\]|\*):8080'; then
    fail "port 8080 is exposed on a non-loopback wildcard address"
else
    pass "port 8080 has no wildcard listener"
fi

if curl --fail --silent --show-error --max-time 10 'http://127.0.0.1:8080/id=3?recommendation' >/dev/null; then
    pass "loopback HTTP health request succeeded"
else
    fail "loopback HTTP health request failed"
fi

if [ "$FAILED" -ne 0 ]; then
    echo "Glennergy verification failed." >&2
    exit 1
fi

echo "Glennergy verification passed."
