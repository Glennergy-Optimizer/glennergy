# VPS systemd cutover checklist

This checklist is the one-time manual migration from the legacy tmux and cron deployment to systemd. Run it during a maintenance window from the Ubuntu VPS. Read the entire checklist and prepare the rollback material before stopping production.

Do not paste the whole document into a shell. Run and review one command group at a time.

## 1. Prepare and identify the release

Build as the normal deployment user, not as root:

```bash
cd /path/to/glennergy
git status --short
git rev-parse HEAD
make
make check-runtime-dependencies
```

Stop here if the worktree is unexpected, the build fails, or preflight reports a missing runtime library.

Record the commit ID and the intended maintenance time in the change notes.

## 2. Capture the legacy state

Create a root-only backup directory:

```bash
sudo install -d -m 0700 /root/glennergy-cutover-backup
sudo cp -a /etc/Glennergy-Fastigheter.json /root/glennergy-cutover-backup/
crontab -l > "$HOME/glennergy-user.crontab.before" 2>/dev/null || true
sudo crontab -l > "$HOME/glennergy-root.crontab.before" 2>/dev/null || true
sudo /bin/sh -c 'umask 077; nginx -T > /root/glennergy-cutover-backup/nginx.before.txt 2>&1'
```

Record processes, listeners, and tmux sessions:

```bash
pgrep -a -f 'Glennergy|glennergy' || true
ss -ltnp
tmux ls
```

Copy both saved crontab files into `/root/glennergy-cutover-backup` after confirming they contain no unrelated secrets that should remain in the deployment user's home.

Stop here unless the configuration backup is readable and the old release or Git revision is available for emergency restoration.

## 3. Stop and remove the legacy runtime

Stop the Glennergy tmux process gracefully using its normal interrupt path. Allow it to exit before closing the tmux session. Do not use `kill -9` as the normal procedure.

Confirm that no Glennergy executable remains:

```bash
pgrep -a -f 'Glennergy-(Main|InputCache|Algoritm|Meteo|Spotpris)' || true
```

Review and remove every legacy Glennergy entry from both crontabs, including old entries without a `# Glennergy` marker:

```bash
crontab -e
sudo crontab -e
crontab -l | grep -iE 'glennergy|meteo|spotpris' || true
sudo crontab -l | grep -iE 'glennergy|meteo|spotpris' || true
```

Run the legacy uninstaller using the command belonging to the old release. Verify again that no old process remains.

Inspect the known legacy IPC objects before removing them:

```bash
ls -l \
    /tmp/fifo_meteo \
    /tmp/fifo_spotpris \
    /tmp/fifo_algoritm \
    /tmp/glennergy_cache.sock \
    /tmp/glennergy-meteo.lock \
    /tmp/glennergy-spotpris.lock \
    /dev/shm/algoritm_shm \
    /dev/shm/sem.algoritm_mutex 2>/dev/null || true
```

After confirming these are stale Glennergy objects, remove only these exact paths:

```bash
sudo rm -f \
    /tmp/fifo_meteo \
    /tmp/fifo_spotpris \
    /tmp/fifo_algoritm \
    /tmp/glennergy_cache.sock \
    /tmp/glennergy-meteo.lock \
    /tmp/glennergy-spotpris.lock \
    /dev/shm/algoritm_shm \
    /dev/shm/sem.algoritm_mutex
```

## 4. Prepare the production configuration

Install the backed-up production configuration at the new path. Do not substitute the repository example for production data:

```bash
sudo install -d -m 0750 /etc/glennergy
sudo install -m 0600 \
    /root/glennergy-cutover-backup/Glennergy-Fastigheter.json \
    /etc/glennergy/fastigheter.json
```

The deployment script validates the JSON and assigns final ownership and permissions.

## 5. Deploy systemd services

From the prepared release checkout:

```bash
sudo ./glennergy_install.sh
```

Do not continue past a deployment error. Keep the complete terminal output, especially the printed deployment-backup path and any rollback message.

## 6. Verify the running deployment

Run the read-only verification and inspect systemd state:

```bash
sudo /bin/sh ./glennergy_verify.sh
sudo systemctl status glennergy.target
systemctl list-timers 'glennergy-*'
systemctl --failed
```

Confirm that exactly the expected processes run under the service account:

```bash
pgrep -a -f 'Glennergy-(Main|InputCache|Algoritm|Meteo|Spotpris)'
ps -u glennergy -o pid,ppid,stat,etime,cmd
```

Inspect current logs:

```bash
journalctl -u glennergy-inputcache.service -u glennergy-algorithm.service -u glennergy-server.service --since '-15 minutes'
journalctl -u glennergy-meteo.service -u glennergy-spotpris.service --since '-15 minutes'
```

Validate the local listener and Nginx configuration:

```bash
ss -ltnp 'sport = :8080'
curl --fail --show-error http://127.0.0.1:8080/id=3
sudo nginx -t
```

Test the real public endpoint through Nginx separately, using HTTP or HTTPS as currently configured. Port 8080 must not be reachable directly from another host.

## 7. Observe and reboot-test

Allow enough time to observe at least one Meteo activation and one Spotpris activation, or start each oneshot deliberately while watching its journal:

```bash
sudo systemctl start glennergy-meteo.service
sudo systemctl start glennergy-spotpris.service
journalctl -u glennergy-meteo.service -u glennergy-spotpris.service --since '-15 minutes'
```

When the running checks pass and a brief reboot interruption is acceptable:

```bash
sudo reboot
```

After reconnecting, repeat:

```bash
sudo /bin/sh ./glennergy_verify.sh
systemctl list-timers 'glennergy-*'
systemctl --failed
sudo nginx -t
```

The cutover is complete only after the reboot check and public Nginx request pass.

## Rollback decision

Do not restore cron or tmux while the new target is running; doing so can create duplicate workers.

For an upgrade of an existing systemd deployment, the deploy script attempts to restore the prior installed binaries, units, enablement, and running state automatically. Verify that result before taking further action.

For the first migration from tmux, there is no previous systemd deployment for automatic rollback to restart. If the new deployment cannot be made healthy:

1. Stop and disable `glennergy.target`.
2. Run the new normal uninstaller, which preserves `/etc/glennergy/fastigheter.json`.
3. Restore the old release and `/etc/Glennergy-Fastigheter.json` from the cutover backup.
4. Use the old release's documented installation/start procedure.
5. Restore the saved user and root crontabs only after confirming that no new Glennergy unit or process is running.
6. Verify legacy process and HTTP health.

Record why rollback was required before attempting another migration.
