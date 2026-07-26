# Glennergy troubleshooting

> **Status:** Read-only-first diagnosis for the current `dev` architecture
>
> **Authoritative snapshot:** `dev` at `42798bee227fcd621cbcb0b37c2b5da771210086`
>
> **Last evidence review:** 2026-07-26

Start by observing the whole stack and preserving evidence. Do not restart,
redeploy, edit production configuration or remove IPC files until the failure
has been narrowed down.

This guide covers symptoms and likely boundaries. The
[operations guide](operations.md) owns state-changing recovery, deployment,
rollback, uninstall and purge. The [server architecture](architecture.md)
explains the underlying process and IPC flow.

## First read-only checks

On the intended host, collect current unit, timer and recent journal state:

```bash
sudo systemctl status glennergy.target
systemctl --failed
systemctl list-timers 'glennergy-*'
journalctl -u glennergy.target -u glennergy-inputcache.service -u glennergy-algorithm.service -u glennergy-server.service --since '-15 minutes'
journalctl -u glennergy-meteo.service -u glennergy-spotpris.service --since today
```

Then run the repository's read-only verifier from a trusted matching checkout:

```bash
sudo /bin/sh ./glennergy_verify.sh
```

It checks account and configuration permissions, unit syntax and state,
timers, loopback binding and a local HTTP request. It does not repair or change
the deployment.

Record:

- installed and checked-out Git revisions;
- which unit first failed and its exit status;
- whether the failure began after deploy, reboot, config change or provider
  outage;
- last successful Meteo and Spotpris runs;
- whether the API is unreachable, empty, stale or structurally wrong.

Do not include host addresses, credentials, private keys or sensitive response
bodies in a report.

## The target or a long-running service is inactive

The target requires InputCache, Algorithm and the HTTP server. Their unit files
restart on failure after five seconds but stop rapid crash loops after five
starts in 60 seconds.

Inspect individual units:

```bash
systemctl status glennergy-inputcache.service
systemctl status glennergy-algorithm.service
systemctl status glennergy-server.service
journalctl -u glennergy-inputcache.service -u glennergy-algorithm.service -u glennergy-server.service --since '-15 minutes'
```

Likely boundaries:

- InputCache requires readable property configuration and must create runtime
  FIFOs and the cache socket.
- Algorithm depends on InputCache and retries a failed cache connection after
  five seconds.
- The HTTP server depends on InputCache and Algorithm, and must bind loopback
  port 8080 and open Algorithm shared memory when serving data.
- systemd ordering starts processes in sequence but is not a readiness probe.

Do not manually launch a second binary to “test” the stack; it can contend for
the same socket, FIFO, shared memory or semaphore.

## Configuration is rejected or properties disappear

Inspect metadata without printing sensitive content:

```bash
sudo stat /etc/glennergy/fastigheter.json
```

The verifier expects `root:glennergy` ownership and mode `0640`. The deploy
script validates JSON syntax, but syntax alone does not prove the property
schema is usable.

Current defects and limits matter:

- InputCache and Meteo use different parsers.
- InputCache examines only the first five array entries.
- Malformed objects within those first five are left as counted, zero-filled
  slots rather than being skipped or making InputCache initialization fail.
- Meteo accepts at most five valid entries and skips invalid objects.
- Current IDs must be JSON integers and usable entries require `city`, `lat`,
  `lon` and `electricity_area`; InputCache additionally requires all panel
  fields.
- The checked-in seed has 17 objects. Later objects use string IDs and omit
  `city`, so they are not valid current examples.
- Some parser failures can still produce a successful process status.

Use [property configuration](property-configuration.md) for the exact current
schema and parser behavior. Do not edit the installed file during diagnosis.
If a correction is needed, back it up and follow the approved operational
change procedure.

## Meteo or Spotpris data is missing

Inspect timers, last service runs and provider-related errors:

```bash
systemctl list-timers 'glennergy-*'
systemctl status glennergy-meteo.timer glennergy-spotpris.timer
journalctl -u glennergy-meteo.service -u glennergy-spotpris.service --since today
```

Meteo is scheduled every 15 minutes and Spotpris hourly. Both are one-shot
services with a 45-second start timeout. They require InputCache, open a FIFO,
fetch external JSON and send one native structure.

Possible causes include:

- InputCache was not ready or its FIFO could not be opened;
- the property file was unreadable or no valid Meteo property was accepted;
- outbound DNS/network/TLS access failed;
- the external provider was unavailable or changed its response;
- today's price data was unavailable for one or more areas;
- the process timed out before completing fetch and FIFO delivery.

Do not assume a successful Meteo exit means a property was fetched: the current
loader treats a zero-property set as success and can send an empty structure.
It also ignores failure from the per-property sample parser, so a provider
response with a missing or invalid `minutely_15` object can still produce a
successful exit with no samples for that property.

## HTTP is unreachable

Run the verifier first. Repository source and units expect the application to
listen only on `127.0.0.1:8080`.

If the local verifier succeeds but an external client cannot connect, the
problem lies beyond the repository-proven application boundary. Check the
host's reverse proxy, TLS certificate, DNS and firewall through the authorized
operational process. Do not expose port 8080 publicly as a workaround.

If local verification fails, inspect `glennergy-server.service` and confirm no
other local process owns port 8080. Do not publish the host address or use a
live public endpoint as a documentation test.

## HTTP returns 400, 204 or an empty array

Current routes are temporary. The implemented data form is:

```text
/id=<non-negative-integer>?recommendation
```

The same temporary shape accepts `weather` and `price`. Unsupported paths or
commands return 400. `/` and `/favicon.ico` return 204. A syntactically valid
request for a positive ID absent from the shared snapshot normally returns
HTTP 200 with `[]`.

ID `0` is an unsafe exception: the parser accepts it, and zero-initialized
unused result slots can also have ID `0`. Such a request can match multiple
slots, emit large zero-filled output, and exceed the fixed response buffer.
Do not use ID `0` as a not-found test.

Check the canonical
[Glennergy–ESP interface contract](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/interface-contract.md)
before changing a client or server. The desired command-first route is planned,
not current behavior.

## HTTP data is zero-filled, stale or surprising

A listening server does not prove useful data is ready. The end-to-end path is:

```text
scheduled fetcher -> InputCache -> Algorithm -> shared memory -> HTTP server
```

Current freshness limitations:

- InputCache keeps its live snapshot in memory.
- Dated cache files are not restored into the live snapshot after restart.
- A failed fetch can leave older in-memory data with no client-visible age
  metadata.
- Algorithm retains its last published snapshot when a cache request fails.
- After successful input retrieval, Algorithm recomputes approximately every
  ten seconds.
- The HTTP server returns the current shared-memory contents and does not test
  provider freshness.
- Empty/uninitialized fixed-array slots can appear as zero values.

Inspect the journals in pipeline order: fetchers, InputCache, Algorithm, then
HTTP server. Correlate timestamps rather than restarting every service at once.

The numeric `recommendation[].type` field has unresolved semantics in the
current implementation. Do not diagnose it against a presumed BUY/HOLD/SELL
contract; consult the interface contract and current limitations.

## IPC failures

Expected runtime channels are:

```text
/run/glennergy/meteo.fifo
/run/glennergy/spotpris.fifo
/run/glennergy/cache.sock
/algoritm_shm
/algoritm_mutex
```

InputCache owns the `/run/glennergy` FIFOs and Unix socket. Algorithm owns or
creates the named shared-memory objects used by the HTTP server. The protocols
copy native C/C++ structures and require a compatible five-binary deployment.

Indicators of IPC trouble include cache connection errors, short payload
reads, FIFO open/write timeout, or shared-memory/semaphore open errors.
Algorithm currently ignores `CacheResponse.data_size` and reads its own
expected size. A partial or mixed-version deployment can therefore short-read,
block, or silently misinterpret data without an explicit announced-size
rejection, and is a primary suspect after an update.

Do not delete socket, FIFO, shared-memory or semaphore objects while services
are running. Do not make them world-writable. If recovery requires a full stack
restart or redeploy, use the gated procedure in [operations](operations.md) so
systemd retains ownership and evidence.

## Deployment or rollback failed

The deploy script retains a timestamped backup and prints its exact path. It
attempts rollback after a failure that occurs once installation has begun.

Preserve:

- deployment terminal output;
- the exact backup path printed by that run;
- relevant systemd journals;
- the source revision used to build the attempted release.

Do not combine binaries from different backups or rebuild only one participant
after an IPC structure change. Do not guess backup paths. Follow the
[operations guide](operations.md) before retrying or restoring anything.

## Recovery actions are gated

Only move from observation to recovery after identifying the affected boundary
and confirming the target host. State-changing actions include:

- starting, stopping or restarting the target or a service;
- manually starting a one-shot fetcher;
- changing production configuration;
- deploying or restoring a release;
- removing IPC objects;
- uninstalling or purging.

Purge is destructive and permanently removes configuration, persistent state
and cache. It is never a troubleshooting step. Back up required data and use
the exact commands and safeguards in [operations](operations.md).

Avoid broad `pkill`, routine `kill -9`, manual duplicate processes and public
port exposure. These actions destroy useful evidence or create competing
owners without addressing the underlying dependency.

## Escalation report template

```text
Observed symptom:
First known occurrence:
Installed Git revision:
Failed unit or timer:
Last successful Meteo run:
Last successful Spotpris run:
Verifier result:
Relevant journal errors (redacted):
Recent deploy/config/reboot event:
Read-only checks completed:
Recovery attempted:
```

Redact production addresses, property/customer data, credentials and key
material before sharing the report.
