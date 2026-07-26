# Glennergy development guide

> **Status:** Current development workflow
>
> **Authoritative snapshot:** `dev` at `42798bee227fcd621cbcb0b37c2b5da771210086`
>
> **Last evidence review:** 2026-07-26

This guide covers local source work and validation. Building, deploying and
verifying an installed host are separate operations. For process and IPC
details, see [server architecture](architecture.md). For production lifecycle
commands, see [operations](operations.md).

## Prerequisites

The production build uses C11 and C++20. It requires:

- GNU Make, GCC and G++;
- libcurl development files;
- Jansson development files;
- POSIX threads and Linux/POSIX IPC support;
- Doxygen only when generating API documentation.

The checked-in Makefiles do not install dependencies. Package names vary by
Linux distribution, so this repository does not prescribe one package-manager
command.

## Build the complete release

Build as an ordinary development user from the repository root:

```bash
make
```

The top-level Makefile deliberately lists the production graph rather than
discovering subdirectories. It produces these compatible artifacts:

- `Glennergy-Main`;
- `Cache/Glennergy-InputCache`;
- `Algorithm/Glennergy-Algoritm`;
- `API/Meteocpp/Glennergy-Meteo`;
- `API/Spotpris/Glennergy-Spotpris`.

`API/Meteocpp` is the production weather implementation. `API/Meteo` is a
legacy C implementation and `Client-CPP` is not part of the production build.

The components exchange native structs through FIFOs, a Unix socket and POSIX
shared memory. If an IPC structure changes, rebuild and deploy all five
artifacts together. Mixed builds do not carry a protocol version and may
misinterpret the data.

An ordinary `make` tracks included headers through generated dependency files.
Use a clean rebuild only when intentionally diagnosing stale artifacts:

```bash
make clean
make
```

`make clean` removes local generated build products. It does not operate on an
installed host.

## Debug and focused builds

The root debug target builds a debug variant of the HTTP server only:

```bash
make debug
```

It does not create debug variants of every production component. Individual
component Makefiles can be invoked for focused compilation, but a complete
release should always be rebuilt from the repository root before deployment.

## Local validation

After building, validate that all required install inputs exist:

```bash
make check-install-inputs
```

On a compatible Linux host, also check that the built executables can resolve
their shared libraries:

```bash
make check-runtime-dependencies
```

These targets inspect local build inputs. They do not install files, restart
services or contact production. The repository currently has no top-level
`make test` target, so documentation must not present one as available.

Do not use the production deploy script as a substitute for local compilation
or testing. `glennergy_install.sh` delegates to the privileged deployment
workflow and can stop and replace an installed release.

## Configuration used by an installation

The checked-in [`API/Glennergy-Fastigheter.json`](../API/Glennergy-Fastigheter.json)
is an initial example. Installation copies it to the production configuration
only when `/etc/glennergy/fastigheter.json` is absent. Later deployments
preserve the installed file.

Current consumers require each usable property to provide an integer `id`, a
string `city`, numeric `lat` and `lon`, numeric panel fields, and a string
`electricity_area`. The example contains temporary and inconsistent entries,
and current result structures support only five properties. Treat it as test
data, not a final registration schema.

Never put credentials, private endpoint addresses, customer data, API keys,
Wi-Fi passwords or key material in an example configuration or commit.

## Documentation development

Generate Doxygen output locally when Doxygen is installed:

```bash
doxygen Doxyfile
```

Generated `html/` and `latex/` directories are ignored and are not sources of
truth. Follow the [Doxygen standard](Doxygen_Standard.md) when editing source
comments. Narrative documentation should explain system responsibilities and
flows; Doxygen should describe source-level contracts, ownership and side
effects.

Before submitting documentation changes:

1. compare behavioral claims with current source, Makefiles, scripts and unit
   files;
2. distinguish implemented, partial, temporary and planned behavior;
3. check relative links from the document's directory;
4. render Mermaid diagrams where practical;
5. remove trailing whitespace and accidental secret or host-specific values.

## Development boundaries

The following are local or read-only activities when run in a development
checkout:

- source and Git inspection;
- `make`, focused compilation and `make clean`;
- install-input and dependency checks;
- Doxygen generation;
- static checks and documentation rendering.

The following cross into host operations and should be handled through the
[operations guide](operations.md): deployment, service start/stop/restart,
installed-host verification, uninstall, purge, production configuration
changes, production log inspection and any action against a live endpoint.
