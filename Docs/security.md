# Glennergy security boundaries

> **Status:** Current repository evidence and known gaps
>
> **Authoritative snapshot:** `dev` at `42798bee227fcd621cbcb0b37c2b5da771210086`
>
> **Last evidence review:** 2026-07-26

This document separates controls implemented in Glennergy source and service
files from host controls that this repository cannot verify. It is not a
penetration test or a claim that a deployed system is secure.

## Security at a glance

| Boundary | Current state |
| --- | --- |
| Application listener | IPv4 loopback only |
| Public edge | Expected reverse proxy, but its TLS/firewall/DNS configuration is outside this repository |
| Read API | Unauthenticated and not property-authorized |
| Registration/write API | Not implemented; future trust model unresolved |
| Service isolation | Dedicated account, restricted permissions and systemd hardening |
| Secrets | Must not appear in source, examples, logs or documentation |

Read the detailed sections when changing exposure, permissions, registration,
logging or production operations.

## Data and secret handling

Never retrieve, print, commit, paste into documentation or include in diagrams:

- GitHub Actions secrets such as `OPENAI_API_KEY`;
- SSH private keys or private TLS keys;
- passwords, tokens, session material or Wi-Fi credentials;
- private endpoint addresses or administrative host details;
- production property/customer data or logs containing it.

Document the location and purpose of a secret only when needed, using a
placeholder such as `<SECRET>` or `<LEOP_BASE_URL>`. Public keys and public TLS
certificates are not secret key material, but should still be included only
for a specific operational reason.

Repository examples must use non-production values. The checked-in property
JSON is temporary example data and must not become a store for live
registrations or credentials.

## Repository-proven network boundary

Current source resolves and binds the HTTP listener to IPv4 loopback
`127.0.0.1`; the systemd server unit supplies port 8080. The verifier rejects a
wildcard listener on that port. Service units restrict permitted address
families according to their roles.

This supports the intended boundary:

```text
external client -> host reverse proxy -> loopback Glennergy HTTP server
```

The reverse-proxy configuration is not in this repository. Consequently,
Glennergy source does not prove:

- a public hostname or IP address;
- Nginx configuration or request filtering;
- TLS availability, certificate validity or HTTPS enforcement;
- firewall rules or cloud-provider security groups;
- DNS ownership or routing;
- production port exposure.

Verify those controls on the intended host without publishing its address or
credentials. Security must not depend on an endpoint address remaining secret;
keeping it out of public documentation only reduces unnecessary disclosure.

## Current HTTP trust model

The current server accepts read requests in the temporary form
`/id=<integer>?<command>` for recommendation, weather and price data. It does
not authenticate a client or authorize access to a property. Responses allow
any CORS origin. Valid unknown IDs return an empty JSON array.

These reads should therefore be treated as unauthenticated data access, even
when a reverse proxy limits practical reachability. Do not describe the API as
private, authenticated or tenant-isolated without an independently verified
external control.

The server does not currently implement property registration, request-body
processing, or writes to `/etc/glennergy/fastigheter.json`. The planned
two-way ESP registration path has an unresolved trust model. Before it is
implemented, the design must define at least:

- device identity and credential provisioning;
- authentication and per-property authorization;
- create-versus-update rules, identifier generation or allocation, and the
  property-to-device mapping;
- payload schema, size and range validation;
- duplicate handling and idempotent retries;
- credential rotation and revocation;
- transport security and certificate validation;
- atomic persistence, concurrent updates and recovery;
- rate limiting, audit events and safe error responses.

Accepting arbitrary JSON and appending it to the property file is not an
acceptable registration design.

## Service identity and filesystem controls

The deployment script creates or validates a dedicated `glennergy` system user
and group with `/usr/sbin/nologin`. Installed executables and systemd units are
installed by root. The production configuration is set to `root:glennergy`
with mode `0640`.

The service units use restrictive defaults including:

- `NoNewPrivileges=true`;
- `ProtectSystem=strict`;
- `ProtectHome=true` and `PrivateTmp=true`;
- kernel, control-group, clock and hostname protections;
- restricted namespaces, realtime and SUID/SGID behavior;
- native syscall architecture;
- per-role address-family restrictions.

InputCache owns restricted runtime and cache directories through systemd. IPC
paths are centralized below `/run/glennergy`; configuration is read from
`/etc/glennergy/fastigheter.json`; generated caches are below
`/var/cache/glennergy`.

These controls reduce impact but do not replace API authentication, reverse
proxy hardening, timely patching or least-privilege host administration.

## Internal IPC boundary

Glennergy components exchange native structures through FIFOs, a Unix-domain
socket and POSIX shared memory. These transports are local, but their messages
are not schema-versioned or cryptographically authenticated.

Deploy all five artifacts as a compatible release. Filesystem permissions and
the dedicated service identity are part of the IPC trust boundary. Do not make
`/run/glennergy`, its socket or FIFOs world-writable. A process able to write
these channels may be able to inject or corrupt service data.

The named shared memory and semaphore use fixed names outside
`/run/glennergy`. Their lifecycle and access behavior should be included in
future hardening and recovery testing.

## External provider boundary

Meteo and Spotpris make outbound requests to public providers. Provider data
is untrusted input even when transported over HTTPS. Parsers must continue to
enforce expected JSON types, array limits and sizes. Provider availability,
schema changes and stale cached data must not be confused with authenticated
Glennergy state.

The service units grant network access only to the components that need it.
Provider URLs do not contain repository-defined credentials.

## Logging and error disclosure

Application logging goes to stdout/stderr and is collected by journald. Current
server debug output can include request paths and complete generated response
bodies. As real property data is introduced, those logs may become sensitive.

Operational practice should:

- restrict journal access;
- avoid logging credentials or registration payloads;
- avoid returning internal paths, stack traces or secret values to clients;
- redact sensitive excerpts before sharing issues or reports;
- define retention appropriate to privacy and the host's disk capacity.

Do not use commands intended to search for, print or export secret values as a
documentation validation step.

## Production-sensitive actions

The following require a deliberate operator decision because they change or
expose live state:

- deploy, start, stop or restart services;
- edit `/etc/glennergy/fastigheter.json`;
- inspect or export production journals;
- change reverse-proxy, TLS, DNS, firewall or SSH configuration;
- call live endpoints with real property identifiers;
- restore deployment backups;
- uninstall or purge the stack.

Purge permanently removes configuration, state and cache. Follow the
[operations guide](operations.md) and verify the exact host and backup before
approval.

## Security review checklist

Before exposing a Glennergy deployment beyond the host:

1. confirm the server listens only on loopback;
2. verify the reverse proxy, HTTPS policy and certificate validation;
3. verify firewall and provider-level ingress rules;
4. inventory every unauthenticated route and its data sensitivity;
5. restrict service, configuration, IPC and journal permissions;
6. confirm no secrets or live data are committed;
7. patch the OS, reverse proxy and linked libraries;
8. test backups and rollback without publishing host details;
9. define monitoring and incident-response ownership;
10. repeat the review before enabling future state-changing registration.

See [server architecture](architecture.md) for the implemented component and
IPC layout and [operations](operations.md) for safe lifecycle separation.
