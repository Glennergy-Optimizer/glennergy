# Glennergy systemd migration plan

## Purpose

This document records the agreed plan for migrating Glennergy from its current tmux, runtime-cron, and recursive installation-script setup to a predictable systemd-managed deployment on the Ubuntu VPS.

It describes the observed starting point, confirmed design decisions, intended end state, migration phases, operational workflow, and deliberately deferred improvements. It is a planning document; it does not imply that the migration has already been implemented.

## Observed initial state

### Host environment

The current production host was reported as:

- Ubuntu 24.04.4 LTS (Noble), Linux 6.8, x86-64.
- systemd 255 running as PID 1 with the unified cgroup hierarchy.
- One virtual CPU, approximately 1 GiB RAM, no swap.
- A 9.4 GiB root filesystem with approximately 5.3 GiB free at the time inspected.
- UFW enabled with inbound SSH and HTTP allowed; port 8080 was not allowed publicly.
- Nginx listening publicly on port 80 and proxying to `http://127.0.0.1:8080`.
- Glennergy itself listening on `0.0.0.0:8080`.

The host is suitable for systemd. Its limited memory and storage mean builds should not use uncontrolled parallelism, and journal retention should be bounded deliberately.

### Current runtime model

Glennergy was started interactively inside a tmux session using approximately:

```bash
Glennergy-Main 8080
```

`Glennergy-Main` then:

1. Installed per-user cron entries at runtime.
2. Forked the TCP server work into a child process.
3. Forked and executed `Glennergy-InputCache`.
4. Forked and executed `Glennergy-Algoritm`.
5. Waited for child processes.
6. Attempted to remove cron entries only after the children exited.

The logger implementation also forked a child logger process in each participating executable. This explains why process inspection showed multiple processes with the same executable names; they were not necessarily duplicate application instances.

This model makes graceful cleanup dependent on the parent process reaching the correct shutdown path. A crash, forced kill, VPS restart, blocked child, or tmux-related interruption can leave cron or IPC state behind.

### Current scheduled work

Meteo and Spotpris were invoked every minute through the interactive user's crontab. Each command used:

- `flock` under `/tmp` to prevent concurrent instances.
- `timeout 45s` to bound a blocked process.
- File redirection under `/var/log/glennergy`.

The inspected crontab contained two copies of each job: an older untagged pair and a newer pair carrying `# Glennergy` comments. The existing removal filter only removes tagged entries, so the older pair can survive cleanup. The shared lock limits simultaneous execution, but cron still attempts all duplicate activations.

### Current build and installation model

`glennergy_install.sh` currently:

1. Builds and installs the root Makefile.
2. Uses `find` to discover additional Makefiles.
3. Builds and installs every discovered, non-excluded Makefile.
4. Creates a `glennergy` group and adds the invoking user to it.
5. Creates log and cache directories with group permissions.

Problems identified in this model include:

- The root Makefile can be processed twice.
- Production components are discovered rather than explicitly declared.
- Adding a new Makefile can silently change the installed product.
- Failure partway through installation can leave a partially updated host.
- A normal build, privileged installation, configuration installation, directory creation, and operational setup are coupled together.
- Rebuilding the root Makefile alone does not build all subcomponents, which led to instructions to rerun the complete installer after every code change.

The uninstaller removes selected installed targets but does not comprehensively own cron, running processes, tmux, runtime IPC, cache, logs, group membership, or future systemd units. It also removes the production JSON configuration, while safer uninstall semantics should preserve operator-managed configuration by default.

### Meteo ambiguity

Both of these subprojects build and install a binary named `Glennergy-Meteo`:

- `API/Meteo` — legacy C implementation.
- `API/Meteocpp` — newer C++ implementation.

Because both install to the same destination and the installer discovers Makefiles dynamically, the last installation overwrites the first. The inspected production binary size strongly suggested that the C++ binary happened to be installed, but the build system does not guarantee that outcome.

### Current filesystem layout

The existing deployment uses approximately:

```text
/usr/local/bin/Glennergy-*
/etc/Glennergy-Fastigheter.json
/var/log/glennergy/
/var/cache/glennergy/
/tmp/fifo_meteo
/tmp/fifo_spotpris
/tmp/fifo_algoritm
/tmp/glennergy_cache.sock
/tmp/glennergy-*.lock
```

Runtime IPC uses predictable global paths and some world-writable permissions under `/tmp`. Installation overwrites the JSON configuration, and normal uninstall removes it.

### Previous systemd attempt

The current branch contains `systemd.sh`, but no matching root-level `.service` or `.timer` files for it to copy. Historical Git commits contain earlier units, but they conflict with the current process model and should not be restored unchanged.

Problems in the historical design included:

- Separate systemd units launched Cache and Algorithm while `Glennergy-Main` also launched them, risking duplicate instances.
- Unit and binary names were inconsistent, including `Algorithm` versus `Algoritm`.
- `KillMode=process` could leave forked descendants alive.
- Units expected a `glennergy` user, while installation created only a group.
- IPC remained coupled to `/tmp`.
- Unit installation/enabling and uninstall behavior were incomplete.

## Migration goals

The migration should produce a deployment that is:

- Automatically started after boot.
- Recoverable after an individual process crash.
- Predictable to install, update, inspect, stop, and uninstall.
- Free from runtime modification of user crontabs.
- Protected from overlapping scheduled fetch jobs.
- Explicit about which source directories produce the production binaries.
- Operated as an unprivileged dedicated account.
- Safe for configuration during upgrade and normal uninstall.
- Observable through standard systemd tools and journald.
- Structured so later reliability improvements do not require another operational redesign.

## Confirmed design decisions

### Process ownership

systemd will directly supervise the long-running components as separate services:

```text
glennergy-server.service
glennergy-inputcache.service
glennergy-algorithm.service
```

`Glennergy-Main` will stop acting as the supervisor for InputCache and Algorithm. Runtime cron installation/removal and unnecessary process-management forks will be removed from the application.

### Production Meteo implementation

`API/Meteocpp` is the sole production Meteo implementation. The legacy C implementation may remain in the repository temporarily for reference or tests, but it will be excluded from the normal production build and installation graph.

Only one target will be permitted to provide the installed `Glennergy-Meteo` executable.

### Scheduling

Scheduled work will use systemd timer/oneshot pairs:

```text
glennergy-meteo.timer
└── glennergy-meteo.service

glennergy-spotpris.timer
└── glennergy-spotpris.service
```

The agreed schedules are:

- Meteo: every 15 minutes.
- Spotpris: hourly.

The initial oneshot timeout is:

```ini
TimeoutStartSec=45s
```

A named systemd oneshot service is not launched again while its current activation remains active. This replaces the main overlap-prevention role currently served by `flock`. The timeout bounds a job that hangs on external I/O or IPC.

### Initial reliability scope

The first migration will use the minimal approach:

- Explicit systemd dependencies and ordering.
- A 45-second outer service timeout.
- No stacking of the same oneshot unit.
- Existing IPC design retained, apart from the path relocation.

The following improvement is an explicit follow-up TODO:

> After the minimal systemd migration is stable, consider replacing blocking IPC operations with bounded non-blocking connection/write retries and adding application-level curl connect/total timeouts. Preserve the 45-second systemd timeout as the final outer safety net.

The reliable version would provide more precise errors and faster failure when InputCache is unavailable, but it is not required for the initial cutover.

### Runtime IPC

Runtime FIFOs, Unix sockets, and related transient objects will move from `/tmp` to:

```text
/run/glennergy/
```

systemd will create and own the runtime directory through `RuntimeDirectory=glennergy`. IPC will be available only to the dedicated service account/group rather than using world-writable modes.

### Configuration and state

The production configuration will move to:

```text
/etc/glennergy/fastigheter.json
```

Rules:

- Initial installation may create it from a packaged example/default if it is absent.
- Upgrade must not overwrite an existing production configuration.
- Normal uninstall preserves configuration and persistent state.
- Explicit purge removes configuration and state after a clear operator request.
- The example configuration should be installed separately, for example under `/usr/share/glennergy`.

Generated cache remains under `/var/cache/glennergy`. Persistent non-cache state, if introduced or identified, belongs under `/var/lib/glennergy`.

### Service identity and permissions

Installation will create or validate:

- A dedicated `glennergy` system user.
- A `glennergy` group.
- A disabled interactive login shell.
- Ownership and permissions appropriate to each data category.

Installed executables will remain owned by `root:root`, preventing the running service from modifying the programs systemd will execute.

Expected ownership model:

```text
/usr/local/libexec/glennergy/  root:root
/etc/glennergy/                root:glennergy
/var/lib/glennergy/            glennergy:glennergy
/var/cache/glennergy/          glennergy:glennergy
/run/glennergy/                glennergy:glennergy
```

The service does not require root privileges. Port 8080 is above the privileged-port range.

### Executable installation location

Internal service binaries will be installed under:

```text
/usr/local/libexec/glennergy/
```

Internal components will not be exposed as general interactive commands in `/usr/local/bin` unless a future operator-facing command specifically warrants it.

### Logging

The agreed end state is journald-only logging.

Existing application logging interfaces and call sites remain valuable and will be preserved, including:

```text
LOG_DEBUG
LOG_INFO
LOG_WARNING
LOG_ERROR
```

The forked logger backend and direct application log files will be replaced. The initial implementation should keep the existing API but synchronously emit messages to stdout/stderr using journal-compatible severity prefixes. systemd will attach timestamps, PIDs, unit identity, boot identity, and retention handling.

Native `sd_journal_send()` structured fields may be considered later, but adding a libsystemd development dependency is not required for the initial migration.

Journal persistence and a bounded retention policy should be configured or verified on the VPS. Given the small root disk, journal usage should have an intentional upper limit.

### Build, installation, and deployment separation

The top-level Makefile will explicitly list the production subcomponents. It will not use filesystem discovery to decide what belongs in a release.

Intended responsibilities:

```text
make              Build all selected production components without root.
make test         Run available validation without changing the host.
make install      Copy already-built artifacts into the selected destination.
make uninstall    Remove installed programs/units while preserving config/state.
make purge        Explicitly remove retained Glennergy config/state/cache as defined.
```

Sub-Makefiles should consistently honor `PREFIX`, `DESTDIR`, and relevant install-directory variables. A staged installation using `DESTDIR` must be possible.

`make install` should not silently restart production. A separate deployment script or documented deployment command will orchestrate:

1. Build.
2. Test/validate.
3. Install artifacts.
4. Verify unit files.
5. Reload systemd.
6. Restart or start the intended target.
7. Run health checks.

On the one-CPU, one-GiB VPS, builds should default to non-parallel or `-j1` behavior unless measurements show otherwise.

### Uninstall and purge

Normal uninstall will:

- Stop and disable Glennergy units/timers.
- Remove installed unit files and executables.
- Reload systemd and clear obsolete failed-unit state where appropriate.
- Remove transient runtime state.
- Preserve production configuration and persistent data.

Purge will additionally remove the explicitly documented Glennergy configuration, persistent state, cache, and optionally the dedicated service account/group when safe.

Neither operation should edit unrelated files, processes, cron entries, users, or groups.

### Backward compatibility

The new deployment will not maintain permanent compatibility with:

- Legacy executable paths under `/usr/local/bin`.
- `/etc/Glennergy-Fastigheter.json`.
- `/tmp` IPC paths.
- Runtime cron manipulation.
- tmux supervision.
- Starting the complete stack through `Glennergy-Main 8080`.

The legacy installation will be cleaned up manually once, using the production cutover checklist below. The operator will preserve the existing configuration and record enough old state to permit a controlled rollback. This is cutover safety, not an ongoing compatibility layer or repository script.

### Network exposure

Glennergy will listen only on:

```text
127.0.0.1:8080
```

Nginx remains the public reverse proxy:

```text
Client -> Nginx on 80/443 -> Glennergy on 127.0.0.1:8080
```

UFW should continue to deny public access to port 8080. Binding to loopback provides defense in depth if firewall configuration later changes.

The inspected Nginx configuration currently exposes HTTP port 80 and proxies correctly to `127.0.0.1:8080`. TLS on port 443 is a recommended security follow-up but does not block the systemd migration.

## Target architecture

```text
                         Ubuntu / systemd

Internet
   |
   v
Nginx :80/:443
   |
   v
127.0.0.1:8080
glennergy-server.service
   |
   +-------------------------+
                             v
                  /run/glennergy IPC
                             ^
                             |
            glennergy-inputcache.service
                    ^                ^
                    |                |
      glennergy-meteo.service   glennergy-spotpris.service
               ^                         ^
               |                         |
      glennergy-meteo.timer     glennergy-spotpris.timer

            glennergy-algorithm.service
                    |
                    +---- depends on InputCache readiness/availability
```

All long-running services execute in the foreground and are directly visible to systemd. Scheduled fetchers are short-lived oneshot units. Application output is captured by journald.

## Expected systemd behavior

### Long-running services

Server, InputCache, and Algorithm should generally use:

- `Type=simple` unless explicit readiness notification is added later.
- `User=glennergy` and `Group=glennergy`.
- Absolute `ExecStart` paths under `/usr/local/libexec/glennergy`.
- `Restart=on-failure` with a controlled delay.
- Start-rate limiting to prevent rapid permanent crash loops.
- A finite `TimeoutStopSec` and graceful `SIGTERM` handling.
- journald output.
- `RuntimeDirectory=glennergy` on the appropriate owner unit or target arrangement.
- Incremental security hardening that is verified against actual filesystem/network needs.

Ordering such as `After=` does not prove application readiness. The initial migration may rely on start ordering plus service restart behavior; bounded client retries/readiness integration remain a follow-up reliability improvement.

### Scheduled services

Meteo and Spotpris should use `Type=oneshot` with:

- A 45-second total start timeout.
- InputCache ordering/dependency rules.
- No `Restart=always`; the timer owns the normal recurrence.
- Clear non-zero exit status on failure.
- Persistent timers when missed runs after downtime should be recovered.
- No external `flock` for ordinary timer activation of the same named service.

## Proposed migration phases

### Phase 1: make the build deterministic

- Declare all production components explicitly in the top-level Makefile.
- Select only the C++ Meteo implementation.
- Normalize target names, especially the existing `Algoritm` spelling versus service naming.
- Make build, clean, test, install, uninstall, and purge responsibilities explicit.
- Ensure ordinary builds require no root privileges.
- Support staged installation through `DESTDIR`.

### Phase 2: establish filesystem and identity

- Create the dedicated service account/group behavior.
- Install internal binaries under `/usr/local/libexec/glennergy`.
- Introduce `/etc/glennergy`, `/var/lib/glennergy`, `/var/cache/glennergy`, and `/run/glennergy` ownership rules.
- Change hard-coded configuration and IPC paths.
- Preserve existing production configuration during migration.

### Phase 3: simplify application process ownership

- Remove runtime crontab management from `Glennergy-Main`.
- Stop `Glennergy-Main` from launching InputCache and Algorithm.
- Run the TCP server directly in the foreground.
- Correct signal handling and graceful shutdown for each independent component.
- Ensure no unit is enabled while the old parent still launches the same component.

### Phase 4: replace the logger backend

- Preserve existing logging macros and severity filtering.
- Remove the pipe/fork/file-writer logger process.
- Emit journal-compatible messages synchronously to stdout/stderr.
- Verify unit-specific logs and severity behavior through `journalctl`.
- Configure/verify persistent journal storage and bounded retention on the VPS.

### Phase 5: add systemd units and timers

- Add separate long-running service units.
- Add Meteo and Spotpris oneshot/timer pairs.
- Add an optional `glennergy.target` for operating the complete stack.
- Verify units with `systemd-analyze verify` before installation.
- Test start order, crash restart, stop behavior, timeout behavior, and boot enablement.

### Phase 6: deployment and lifecycle tooling

- Replace the current broad recursive installer with explicit deployment steps.
- Make installation idempotent.
- Implement safe normal uninstall and explicit purge.
- Add health checks after deployment.
- Document operator commands and troubleshooting.

### Phase 7: production cutover

This phase is a one-time manual operator procedure. It will not be implemented as a permanent cutover script in the repository.

The detailed operator procedure is recorded in `Docs/VPS_CUTOVER_CHECKLIST.md`.

- Capture current process, cron, configuration, Nginx, and installed-file state.
- Build and validate the selected release.
- Stop the tmux-managed stack.
- Remove every Glennergy cron entry, including older untagged entries.
- Verify no old Glennergy process remains.
- Remove stale Glennergy IPC objects under `/tmp`.
- Back up `/etc/Glennergy-Fastigheter.json`, then install it as `/etc/glennergy/fastigheter.json` without overwriting an existing new-layout configuration.
- Install and enable the systemd deployment.
- Verify unit status, timers, journald, IPC, HTTP through Nginx, and reboot behavior.

## Cutover and rollback expectations

Permanent legacy compatibility is not required, but cutover must be reversible during initial validation.

Before cutover:

- Back up the production JSON configuration.
- Record the currently installed executable versions/checksums if practical.
- Record current cron and Nginx state.
- Keep the prior build or release available temporarily.

A rollback should explicitly:

1. Stop/disable the new target and timers.
2. Restore the prior binaries/configuration as necessary.
3. Restore the prior scheduler/runtime method only if an emergency rollback truly requires it.
4. Verify process and HTTP health.

Checking out an older Git commit alone is not considered a complete deployment rollback because it does not restore installed artifacts, host configuration, permissions, services, or runtime state.

## Operational end state

Expected routine commands include:

```bash
sudo systemctl status glennergy.target
sudo systemctl start glennergy.target
sudo systemctl stop glennergy.target
sudo systemctl restart glennergy.target
systemctl list-timers 'glennergy-*'
journalctl -u glennergy-server.service
journalctl -f -u glennergy-inputcache.service
sudo /bin/sh ./glennergy_verify.sh
```

A normal code update should resemble:

```text
Update source
-> build all declared components
-> run tests/validation
-> install artifacts
-> reload/restart affected units
-> run health check
```

It should not reinstall dependencies, recreate accounts, overwrite configuration, rediscover production components, or modify cron on every code change.

## Follow-up improvements not required for initial migration

The following items are valuable but deliberately deferred:

- Bounded non-blocking IPC open/connect/write retries.
- Curl connect and total-operation timeouts with precise error reporting.
- Explicit service readiness using `Type=notify` or another readiness mechanism.
- Replacing FIFOs with a Unix socket protocol or socket activation.
- Native structured journald calls through `sd_journal_send()`.
- Versioned release directories and atomic symlink-based rollback.
- CI-built artifacts or Debian packaging instead of compiling on the VPS.
- A small emergency swap file for the low-memory VPS.
- TLS/HTTPS configuration in Nginx.
- External uptime monitoring and alerting.
- Backup restoration exercises.
- Stronger systemd sandboxing after required filesystem and network access is measured.

## Acceptance criteria

The initial migration is complete when all of the following are true:

- No Glennergy component requires tmux to remain alive.
- Reboot automatically restores the intended services and timers.
- `Glennergy-Main` no longer launches Cache, Algorithm, or cron scripts.
- Exactly one production instance of each long-running component is supervised by systemd.
- Meteo runs every 15 minutes and Spotpris hourly without overlapping instances.
- A blocked scheduled job is terminated after 45 seconds.
- Killing a long-running component causes the intended controlled restart without creating duplicates.
- Stopping `glennergy.target` leaves no Glennergy service processes behind.
- No Glennergy cron entries remain.
- Production uses the C++ Meteo implementation deterministically.
- Runtime IPC is contained under `/run/glennergy` with restricted permissions.
- Glennergy runs as the dedicated non-login account and not as root.
- Glennergy listens only on `127.0.0.1:8080`; Nginx remains the public entry point.
- Existing configuration survives upgrade and normal uninstall.
- Journald contains the existing application messages with useful component and severity information.
- Build, install, deploy, uninstall, and purge have distinct documented behavior.
- Installation can be repeated without corrupting ownership, configuration, or scheduler state.

## Decision record summary

| Topic | Decision |
|---|---|
| Long-running process supervision | Separate systemd services |
| Production Meteo | C++ implementation only |
| Meteo schedule | Every 15 minutes |
| Spotpris schedule | Hourly |
| Scheduled-job timeout | 45 seconds |
| Initial IPC reliability | Minimal dependency/timeout approach |
| Later IPC reliability | Bounded retry/timeouts recorded as TODO |
| Runtime IPC directory | `/run/glennergy` |
| Configuration | `/etc/glennergy/fastigheter.json`, preserved by default |
| Service identity | Dedicated non-login `glennergy` user/group |
| Internal binaries | `/usr/local/libexec/glennergy` |
| Logging | Journald-only; preserve application logging API |
| Build graph | Explicit top-level component orchestration |
| Deployment | Separate from ordinary build/install |
| Removal | Distinct uninstall and purge |
| Backward compatibility | No permanent legacy compatibility |
| Application listener | `127.0.0.1:8080` |
| Public entry point | Nginx reverse proxy |
