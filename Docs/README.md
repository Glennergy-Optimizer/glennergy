# Glennergy documentation

> **Status:** Documentation index for the current development architecture
>
> **Authoritative snapshot:** `dev` at `42798bee227fcd621cbcb0b37c2b5da771210086`
>
> **Stable-production reference:** `origin/main` at `61761b5eda30bee417a0b6e33e10fb061e18db26`
>
> **Last reviewed:** 2026-07-26

This is the canonical index for server-specific Glennergy documentation.
Glennergy-ESP owns the shared system context, terminology, interface contract
and cross-project limitations. Glennergy owns the server implementation,
configuration, development, operations, security and troubleshooting guides.

The documentation campaign describes `dev`, the authoritative implementation
branch. `main` represents stable production, but the two branches currently
differ substantially. A page must explicitly say when its instructions also
apply to stable production; do not infer production behavior from an unmarked
`dev` guide.

## Start here

| If you want to… | Read | Classification |
|---|---|---|
| Understand what the server does and get started | [Project README](../README.md) | Current entry point; being revised during this campaign |
| Understand server processes, ownership, IPC and refresh timing | [Server architecture](architecture.md) | Current canonical server architecture for `dev` |
| Understand how Glennergy and Glennergy-ESP fit together | [System context](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/system-context.md) | Canonical cross-project overview, owned by Glennergy-ESP |
| Implement or debug the current HTTP exchange | [Interface contract](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/interface-contract.md) | Canonical cross-project contract, owned by Glennergy-ESP |
| Check incomplete, temporary or planned behavior | [Current limitations](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/current-limitations.md) | Canonical cross-project status, owned by Glennergy-ESP |

The public GitHub links above are repository-document links, not deployment
endpoints. During branch review they may return 404 until the corresponding
Glennergy-ESP documentation is available on `dev`; in a local sibling checkout,
open the same paths under `Glennergy-ESP/docs/`. No production hostname or VPS
address is published here.

## Current canonical server documentation

### Server architecture

[Server architecture](architecture.md) is the current technical description of
the five production executables, their data ownership, FIFO and Unix-socket
messages, shared memory and semaphore, systemd schedules, startup readiness and
failure behavior. It is the source to update when process boundaries, IPC
layouts, service ordering or refresh schedules change.

### Root README

The [root README](../README.md) remains the approachable entry point for new
developers and operators. Its role is to explain purpose, prerequisites, safe
build and deliberate deployment, routine operation, troubleshooting entry
points and links into detailed documentation. It should summarize rather than
duplicate the interface contract or server architecture.

## Server guides

The following artifacts are approved but not yet present. Until they are
written and reviewed, source code, systemd units and scripts remain the
implementation evidence; older prose must not be treated as a canonical
substitute.

| Planned path | Audience and scope | Required distinction |
|---|---|---|
| [Property configuration](property-configuration.md) | Maintainers configuring properties; schema, paths, parser behavior and examples | Current integer property IDs and temporary capacity versus planned UUID-like device identity and registration |
| [Development](development.md) | Server developers; prerequisites, builds and safe static checks | A successful build is not deployment or runtime proof; there is no unified root test target |
| [Operations](operations.md) | Operators; install/update, services, timers, logs, verification, backup, rollback, uninstall and purge | Read-only/routine actions versus production-sensitive or destructive actions |
| [Security](security.md) | Developers and operators; loopback boundary, service permissions, systemd hardening and secret handling | Repository-proven behavior versus unverified reverse-proxy/TLS/firewall state; current unauthenticated reads versus requirements for future writes |
| `Docs/troubleshooting.md` | Developers and operators; services, timers, IPC, freshness, API and deployment diagnosis | Read-only diagnosis first; destructive recovery is never a default step |

## API reference and Doxygen tooling

Function-, type- and module-level reference is generated from source comments
with [Doxygen configuration](../Doxyfile). Generated pages describe code APIs;
they do not replace the system architecture, interface contract or operational
guides.

| Document or output | Classification | Use |
|---|---|---|
| [Doxygen standard](Doxygen_Standard.md) | Contributor tooling; retained, with portability review still required | Comment content and formatting conventions |
| [Doxygen workflow guide](Doxygen_Workflow_Guide.md) | Contributor tooling; retained, with portability review still required | Local generation and review workflow |
| [Doxygen TODO](Doxygen_TODO.md) | Tooling backlog, not product status | Documentation-tool maintenance tasks |
| [`tools/doxygen_ai/`](../tools/doxygen_ai/) | Contributor automation | Assisted source-comment workflow; generated claims still require code review |
| [`html/`](../html/) and [`latex/`](../latex/) | Generated, derived and noncanonical | Locally generated API output; regenerate from source rather than hand-editing |

Source and reviewed comments are authoritative over generated output. Generated
directories may be stale relative to the checked-out code and should not be
used to establish deployment state.

## Historical and noncanonical material

These files remain useful evidence of project history or earlier thinking, but
they do not override current code or canonical documentation.

| Document | Classification | How to use it |
|---|---|---|
| [SYSTEMD_MIGRATION_PLAN.md](../SYSTEMD_MIGRATION_PLAN.md) | Historical/mixed migration plan | Some planned work is now implemented and some prose may still describe transitional state. Use current units plus architecture/operations documentation for present behavior. |
| [AI_CONTEXT.md](../AI_CONTEXT.md) | Stale contributor/agent context | Useful for orientation only after verifying every claim against `dev`; intended to be updated or replaced by maintained contributor guidance. |
| `Docs/architecture-overview.md` | Superseded, noncanonical overview | Replaced by [server architecture](architecture.md). It may exist in local or ignored worktrees but is not the maintained architecture source. |
| Legacy planning notes, duplicate root-level documentation copies and commented implementations | Historical or cleanup candidates | Preserve only when they explain a decision; do not copy their claims into current guides without verification. |

No historical file is approved for deletion merely because it is classified
here. Cleanup requires a separate, evidence-backed change.

## Documentation ownership

| Change | Update at minimum |
|---|---|
| Add, remove or reorder a server process | This index, server architecture and root README |
| Change an internal struct, IPC path or semaphore | Server architecture, Doxygen reference and deployment compatibility notes |
| Change a route, response schema, status or ESP parser | Glennergy-ESP interface contract and affected server/firmware docs |
| Change property schema or current identity rules | Property-configuration guide, interface contract when externally visible, and limitations |
| Change a service, timer, install path or operational script | Operations guide, server architecture when topology changes, and root README |
| Change authentication, network exposure or secret handling | Security guide, interface contract and limitations |
| Resolve a known temporary or planned behavior | Glennergy-ESP limitations plus every affected repository-specific guide |

Documentation claims should identify whether they are implemented, partial,
temporary, planned or unknown. Planned behavior—especially property
registration, UUID-like device identity and authorization—must not be presented
as a current server capability.
