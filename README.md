# Glennergy

Glennergy fetches weather and electricity-price data and calculates optimized energy usage. The production stack runs on Ubuntu under systemd; Nginx proxies public HTTP traffic to the server on `127.0.0.1:8080`.

## Production components

The stack consists of three long-running services and two scheduled jobs:

- `glennergy-inputcache.service`
- `glennergy-algorithm.service`
- `glennergy-server.service`
- `glennergy-meteo.timer` and `glennergy-meteo.service`
- `glennergy-spotpris.timer` and `glennergy-spotpris.service`

`glennergy.target` starts and stops the complete stack. Meteo uses the C++ implementation under `API/Meteocpp`.

## Build and initial installation

Build as the normal development user. Do not build as root:

```bash
git clone https://github.com/keseboleliasteliacom/glennergy.git
cd glennergy
make
```

Review `API/Glennergy-Fastigheter.json` before the first installation. It is used only when `/etc/glennergy/fastigheter.json` does not already exist.

Deploy the completed build as root:

```bash
sudo ./glennergy_install.sh
```

The deployment first verifies that every required binary, unit, and configuration source exists, before it stops an installed release. It then creates the non-login `glennergy` service account, installs the artifacts, validates the units, starts the stack, and performs health checks. It preserves the existing production configuration and retains a deployment backup under `/var/backups/glennergy`.

## One-time migration from tmux and cron

The legacy cleanup is deliberately a manual operator procedure and is not part of the reusable installation script.

Before changing the running VPS:

1. Back up `/etc/Glennergy-Fastigheter.json`, the current crontabs, and the Nginx configuration.
2. Record the running Glennergy processes and keep the previous release available.
3. Build and validate the new release before stopping production.
4. Stop the tmux-managed Glennergy processes gracefully.
5. Remove the old Glennergy Meteo and Spotpris cron entries from both the deployment user's and root's crontabs.
6. Run the legacy uninstaller and verify that no old Glennergy process remains.
7. Remove only confirmed stale Glennergy IPC objects left under `/tmp` or `/dev/shm`.
8. Copy the backed-up production JSON to `/etc/glennergy/fastigheter.json` before running the new installer.

The new configuration can be prepared without depending on the service account:

```bash
sudo install -d -m 0750 /etc/glennergy
sudo install -m 0600 /path/to/fastigheter.backup.json /etc/glennergy/fastigheter.json
sudo ./glennergy_install.sh
```

The deployment script assigns the final `root:glennergy` ownership and `0640` permissions.

## Updating production

Create and validate a complete build first, then deploy it:

```bash
make
sudo ./glennergy_install.sh
```

The Makefiles track included headers, so an ordinary `make` rebuilds targets affected by source or header changes. Use `make clean && make` only when you intentionally need a completely fresh rebuild or are diagnosing stale local artifacts.

The deploy script serializes deployments, backs up the currently installed release, installs the new artifacts, reloads systemd, restarts the target, and checks service and HTTP health. If deployment fails after installation begins, it attempts to restore the previous installed binaries, units, and target state.

The deployment backup is not a source build. Keep the corresponding Git revision or release information so the installed state can be reproduced later.

## Routine operation

```bash
sudo systemctl status glennergy.target
sudo systemctl start glennergy.target
sudo systemctl stop glennergy.target
sudo systemctl restart glennergy.target
systemctl list-timers 'glennergy-*'
```

Inspect logs through journald:

```bash
journalctl -u glennergy-server.service
journalctl -f -u glennergy-inputcache.service
journalctl -u glennergy-meteo.service --since today
journalctl -u glennergy-spotpris.service --since today
```

Check the loopback HTTP endpoint directly on the VPS:

```bash
curl --fail --show-error http://127.0.0.1:8080/id=3
```

Public requests should continue through Nginx rather than exposing port 8080 externally.

Run the complete read-only host verification after deployment and maintenance:

```bash
sudo /bin/sh ./glennergy_verify.sh
```

It checks the service identity, configuration permissions, installed unit syntax, boot enablement, active services and timers, loopback-only port binding, and an HTTP request. It reports failures without starting, stopping, or modifying the deployment.

## Uninstall and purge

Normal uninstall stops and disables the stack and removes installed binaries and units. It preserves configuration and state:

```bash
sudo /bin/sh ./glennergy_uninstall.sh uninstall
```

Purge permanently removes Glennergy configuration, state, and cache. It also removes the service account and removes the group only when no other account uses it:

```bash
sudo /bin/sh ./glennergy_uninstall.sh purge --confirm
```

Back up required production data before purging.

## Troubleshooting

Show the complete stack and recent errors:

```bash
sudo systemctl status glennergy.target
systemctl --failed
journalctl -u glennergy.target -u glennergy-inputcache.service -u glennergy-algorithm.service -u glennergy-server.service --since '-15 minutes'
```

Check scheduled jobs:

```bash
systemctl list-timers 'glennergy-*'
systemctl status glennergy-meteo.timer glennergy-spotpris.timer
journalctl -u glennergy-meteo.service -u glennergy-spotpris.service --since today
```

Do not use `kill -9`, broad `pkill` commands, or manual process spawning as routine recovery. Stop or restart the relevant systemd unit so systemd retains ownership of process state and records the result.

If a deployment reports rollback, inspect its terminal output and the journal before retrying. Backups are stored under `/var/backups/glennergy`; restoration should use the exact backup path printed by that deployment.

## Development documentation

Generate the Doxygen documentation when Doxygen is installed:

```bash
doxygen Doxyfile
```

Open the generated `html/index.html` file. The project documentation standard is described in `Docs/Doxygen_Standard.md`.

## Installation layout

```text
/usr/local/libexec/glennergy/       Installed executables
/usr/local/share/glennergy/         Example configuration
/etc/glennergy/fastigheter.json     Production configuration
/var/lib/glennergy/                 Persistent application state
/var/cache/glennergy/               Application cache
/run/glennergy/                     Runtime IPC
/etc/systemd/system/glennergy*      Target, services, and timers
/var/backups/glennergy/             Deployment backups
```

See `SYSTEMD_MIGRATION_PLAN.md` for the architecture decisions, migration rationale, deferred reliability improvements, and acceptance criteria.
