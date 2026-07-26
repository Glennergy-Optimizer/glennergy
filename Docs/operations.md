# Glennergy operations guide

> **Status:** Repository-defined `dev` operations
>
> **Authoritative snapshot:** `dev` at `42798bee227fcd621cbcb0b37c2b5da771210086`
>
> **Last evidence review:** 2026-07-26

This guide describes the lifecycle implemented by the checked-in Makefiles,
deployment scripts and systemd units. It does not prove the current state of
any external host. Production addresses are intentionally omitted.

For local compilation, see [development](development.md). For the trust and
network boundaries, see [security](security.md).

## Operational model

The repository defines three long-running services and two scheduled jobs:

- `glennergy-inputcache.service`;
- `glennergy-algorithm.service`;
- `glennergy-server.service`;
- `glennergy-meteo.timer` and its one-shot service;
- `glennergy-spotpris.timer` and its one-shot service.

`glennergy.target` groups the stack. The long-running services use
`Restart=on-failure` with a five-second delay. Meteo runs every 15 minutes and
Spotpris hourly; each one-shot service has a 45-second start timeout.

The server unit starts `Glennergy-Main 8080 1`. Current source binds the IPv4
listener to `127.0.0.1`. The repository does not include the external Nginx,
TLS, DNS or firewall configuration, so those must be verified separately on
the intended host.

## Build, deploy and verify are separate

### 1. Build as a normal user

```bash
make
make check-install-inputs
make check-runtime-dependencies
```

These commands prepare and inspect local artifacts. They do not update an
installed release.

### 2. Deploy as root

Before running a deployment command, confirm the exact intended host and your
authorization, the revision and complete artifacts being installed, whether
current configuration/data need an additional backup, and that service
downtime is acceptable. Deployment stops the existing target while replacing
the release.

```bash
sudo ./glennergy_install.sh
```

This is a production-sensitive operation. The wrapper executes
`glennergy_deploy.sh`, which:

- serializes deployment with `/run/lock/glennergy-deploy.lock`;
- validates build artifacts, shared-library resolution and JSON syntax;
- creates or validates the non-login `glennergy` service account;
- records installed binaries, units and configuration in a timestamped backup
  under `/var/backups/glennergy`;
- stops an existing `glennergy.target`;
- installs the complete release and systemd units;
- preserves an existing `/etc/glennergy/fastigheter.json`;
- verifies unit syntax, enables and starts the target;
- checks required units and a loopback listener;
- attempts automatic rollback if deployment fails after installation begins.

The retained backup is an installed-state backup, not a reproducible source
build. Record the deployed Git revision separately.

### 3. Verify the installed host

```bash
sudo /bin/sh ./glennergy_verify.sh
```

The verifier is designed to be read-only. It checks the service account,
configuration ownership and mode, installed unit syntax, boot enablement,
active services and timers, loopback-only port binding, and a local HTTP
request. It reports failure without starting, stopping or rewriting the
installation.

The deploy script checks unit activity and listener readiness itself, but the
separate verifier performs the payload-level HTTP check. A successful listener
or HTTP request does not prove that provider data is fresh.

## Routine inspection

Production journals can contain property data, request paths, or generated
response bodies. Confirm authorization before reading them, and redact
sensitive details before sharing output.

These commands inspect state without intentionally changing it:

```bash
sudo systemctl status glennergy.target
systemctl list-timers 'glennergy-*'
systemctl --failed
journalctl -u glennergy-server.service --since today
journalctl -u glennergy-inputcache.service --since today
journalctl -u glennergy-algorithm.service --since today
journalctl -u glennergy-meteo.service --since today
journalctl -u glennergy-spotpris.service --since today
```

## Service control

Before changing service state, confirm the exact host, authorization, expected
impact, current revision, and acceptable downtime. Starting, stopping, or
restarting the target changes live production state.

These commands change running production state:

```bash
sudo systemctl start glennergy.target
sudo systemctl stop glennergy.target
sudo systemctl restart glennergy.target
```

Use systemd rather than manually spawning binaries or using broad `pkill`,
`kill -9` or tmux-based supervision. That preserves service ownership,
dependency handling, restart accounting and journald context.

Starting the target does not guarantee immediate data readiness. InputCache
must open its IPC endpoints, fetchers must deliver inputs, and Algorithm must
publish a shared-memory snapshot. Ordering in the unit files is not an
application readiness handshake.

## Troubleshooting by symptom

### Stack or component is inactive

```bash
sudo systemctl status glennergy.target
systemctl --failed
journalctl -u glennergy.target -u glennergy-inputcache.service -u glennergy-algorithm.service -u glennergy-server.service --since '-15 minutes'
```

InputCache and Meteo require a readable
`/etc/glennergy/fastigheter.json`. Check the journal before restarting.

### Scheduled data is missing or old

```bash
systemctl list-timers 'glennergy-*'
systemctl status glennergy-meteo.timer glennergy-spotpris.timer
journalctl -u glennergy-meteo.service -u glennergy-spotpris.service --since today
```

InputCache primarily owns live data in memory. Its dated JSON cache files are
not loaded back into the live snapshot at startup. After a restart, useful API
data may therefore wait for producer and Algorithm cycles.

### Listener is unavailable

Run the repository verifier. Current code and verification expect only a
loopback listener on port 8080. Do not expose that application port publicly
as a troubleshooting shortcut.

### Deployment reports rollback

Inspect the deployment output and relevant journals. The script prints the
exact retained backup path. Do not guess a backup directory or manually mix
files from different builds; native-structure IPC requires a compatible
release set.

## Installed layout

```text
/usr/local/libexec/glennergy/       Service executables
/usr/local/share/glennergy/         Example configuration
/etc/glennergy/fastigheter.json     Production configuration
/var/lib/glennergy/                 Persistent application state
/var/cache/glennergy/               Generated provider caches
/run/glennergy/                     Runtime FIFOs and Unix socket
/etc/systemd/system/glennergy*      Installed target, services and timers
/var/backups/glennergy/             Deployment backups
```

## Removal

### Normal uninstall

```bash
sudo /bin/sh ./glennergy_uninstall.sh uninstall
```

This is production-sensitive and service-disrupting. It stops and disables the
stack and removes installed executables and units, while preserving
configuration and application data.

### Purge

```bash
sudo /bin/sh ./glennergy_uninstall.sh purge --confirm
```

Purge is destructive. It removes Glennergy configuration, persistent state and
cache, then removes the service account and its group when safe. Back up data
and resolve the exact target host before approving it. The command cannot be
undone from the repository alone.

## Host-dependent checks

The repository establishes the intended application-side boundary, but an
operator must independently verify:

- which revision is installed;
- whether the reverse proxy is configured and healthy;
- TLS certificate validity and HTTPS-only policy;
- DNS and public hostname routing;
- firewall exposure, especially that application port 8080 is not public;
- backup retention and restoration;
- journal retention and disk capacity;
- external monitoring and alerting.

Do not put real hostnames, IP addresses, credentials or private keys in this
guide when recording the result.
