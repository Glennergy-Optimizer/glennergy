# Glennergy

Glennergy is the LEOP server for the Glennergy system. It fetches weather and
Swedish electricity-price data, combines those inputs for each configured
property, and serves the latest calculated data to Glennergy-ESP devices over
HTTP.

The project is close to feature-complete, but some behavior remains temporary
or planned. In particular, property data is currently preconfigured, device
registration is not implemented, and the exact meaning of the numeric
recommendation value remains unresolved. See the
[current limitations](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/current-limitations.md)
before building a client around unfinished behavior.

> Documentation describes the authoritative `dev` implementation at
> `42798bee227fcd621cbcb0b37c2b5da771210086`. `main` represents the stable
> production line, but currently predates parts of the documented `dev`
> deployment. Do not assume a `dev` operational detail is installed in
> production without verifying the host and revision.

## How the projects fit together

- **Glennergy** runs the server-side fetch, cache, calculation and HTTP stack.
- **Glennergy-ESP** runs on the ESP32-S3 and consumes Glennergy data for its UI.
- Property registration from an ESP back to Glennergy is planned but not yet
  implemented. Current properties come from a server-side JSON file.

The shared [system context](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/system-context.md)
is maintained in Glennergy-ESP. Glennergy owns the complete
[server HTTP API reference](Docs/http-api.md), while Glennergy-ESP maintains a
synchronized [consumer-facing interface contract](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/interface-contract.md).
These links are repository documentation, not live endpoints.

## Runtime components

Glennergy builds five cooperating executables:

| Component | Responsibility | Runtime model |
|---|---|---|
| InputCache | Owns the combined property, weather and price input snapshot | Long-running service |
| Meteo | Fetches 15-minute weather forecasts from Open-Meteo | One-shot job every 15 minutes |
| Spotpris | Fetches prices for Swedish areas SE1–SE4 | One-shot job hourly |
| Algorithm | Reads InputCache and publishes the current per-property result snapshot | Long-running service |
| HTTP server | Serves the latest result snapshot on IPv4 loopback | Long-running service |

systemd supervises the stack through `glennergy.target`. The production weather
implementation is the C++ module under `API/Meteocpp`. For process ownership,
IPC and refresh timing, read the [server architecture](Docs/architecture.md).

## Build safely

The production build requires a Linux/POSIX environment, GNU Make, GCC and G++
with C11/C++20 support, and the development files for libcurl and Jansson.
Doxygen is optional.

Build as a normal user:

```bash
git clone https://github.com/Glennergy-Optimizer/glennergy.git
cd glennergy
make
make check-install-inputs
make check-runtime-dependencies
```

This builds and inspects local artifacts. It does not install them or change a
running service. The repository currently has no top-level `make test` target.
See the [development guide](Docs/development.md) for focused builds, clean
rebuilds, documentation generation and validation boundaries.

## Configure properties

[`API/Glennergy-Fastigheter.json`](API/Glennergy-Fastigheter.json) is an initial
example, not the final registration model. A first installation may copy it to
`/etc/glennergy/fastigheter.json`; later deployments preserve the installed
configuration.

The example currently contains more entries than the end-to-end five-property
test limit and includes entries that do not satisfy the implemented schema.
Review the [property-configuration guide](Docs/property-configuration.md) before
using or changing it. Never place credentials or real private data in the
checked-in example.

## Deploy deliberately

Deployment is a separate, privileged and service-disrupting operation. After a
complete build and review, first confirm the exact intended host and your
authorization, the revision and complete artifacts, backup needs, and
acceptable downtime. Then an authorized operator can run:

```bash
sudo ./glennergy_install.sh
```

The deployment validates inputs, preserves the current production
configuration, backs up the installed release, installs the complete compatible
artifact set, verifies systemd units, starts the stack and attempts rollback on
failure. It must not be used as an ordinary development build command.

Follow the [operations guide](Docs/operations.md) for deployment, read-only host
verification, routine inspection, service control, rollback, uninstall and
purge. The repository proves that the application binds to `127.0.0.1:8080`;
it does not contain or verify the external reverse proxy, TLS, DNS or firewall
configuration. See [security boundaries](Docs/security.md).

## Current API note

The current read endpoint shape is temporary and backwards:

```text
/id=<integer>?recommendation
```

Weather and price commands also exist. Reads are currently unauthenticated,
and the server does not implement property registration or configuration
writes. Do not infer a settled business meaning from the numeric
`recommendation[].type` field. The exact implemented schemas, error behavior
and planned canonical route are documented in the local
[HTTP API reference](Docs/http-api.md). The
[ESP interface contract](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/interface-contract.md)
adds firmware parser, cache, retry and compatibility details.

No real hostname, VPS address, credential or private key is published here.

## Routine navigation

| Need | Document |
|---|---|
| Find the right document | [Documentation index](Docs/README.md) |
| Understand processes and data flow | [Server architecture](Docs/architecture.md) |
| Use or implement the HTTP API | [HTTP API reference](Docs/http-api.md) |
| Build and validate locally | [Development](Docs/development.md) |
| Configure current properties | [Property configuration](Docs/property-configuration.md) |
| Deploy, verify or operate a host | [Operations](Docs/operations.md) |
| Understand trust and exposure | [Security](Docs/security.md) |
| Diagnose a failure safely | [Troubleshooting](Docs/troubleshooting.md) |
| Read function/type reference | Generate Doxygen with `doxygen Doxyfile` and follow the [Doxygen standard](Docs/Doxygen_Standard.md) |

Start troubleshooting with inspection, not recovery commands. Do not use
manual process spawning, broad `pkill`, `kill -9`, purge, or production config
edits as default diagnostics.
