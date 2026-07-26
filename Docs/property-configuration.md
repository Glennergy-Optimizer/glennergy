# Property configuration

> **Status:** Current implementation plus explicitly separated planned work
>
> **Authoritative snapshot:** `dev` at `42798bee227fcd621cbcb0b37c2b5da771210086`
>
> **Last evidence review:** 2026-07-26

Glennergy currently reads property information from a local JSON file. It does
not currently register ESP32-S3 units, accept property writes over HTTP, or
authenticate a device identity. The five-property capacity is a temporary test
limit rather than a final product limit.

For server-visible identity and request behavior, see the
[HTTP API reference](http-api.md). For firmware consumption and compatibility,
see the synchronized
[Glennergy-ESP interface contract](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/interface-contract.md).

## Files and deployment behavior

| Purpose | Path | Behavior |
|---|---|---|
| Repository seed | [`API/Glennergy-Fastigheter.json`](../API/Glennergy-Fastigheter.json) | Example/initial configuration shipped with the repository |
| Installed runtime configuration | `/etc/glennergy/fastigheter.json` | Read by InputCache and Meteo |
| Installed example | `/usr/local/share/glennergy/fastigheter.example.json` | Copy of the repository seed for reference |

`make install` creates the runtime file from the seed only when the runtime file
does not already exist. A normal deployment preserves an existing runtime
configuration, validates that it is syntactically valid JSON, backs it up, and
sets ownership and permissions. Syntactically valid JSON is not necessarily a
valid Glennergy property configuration. The readers perform the field checks
described below, but current error-propagation defects mean process success
does not always prove that a usable property was loaded.

The configuration is not watched for changes. InputCache loads it when the
long-running cache process starts. The Meteo oneshot loads it on each run.
Restart or rerun the relevant processes through the approved operational
procedure after changing the installed file.

## Current JSON shape

The root must be an object containing a `systems` array:

```json
{
  "systems": [
    {
      "id": 3,
      "city": "example-city",
      "lat": 55.61,
      "lon": 13.02,
      "panel_capacitykwh": 6.0,
      "panel_tiltdegrees": 35.0,
      "panel_azimuthdegrees": 180.0,
      "electricity_area": "SE4"
    }
  ]
}
```

This example uses non-production values and no server address.

### Fields accepted by the current readers

| Field | Current type | InputCache reader | Meteo reader | Current use |
|---|---|---|---|---|
| `id` | JSON integer | Required | Required | Copied through weather/cache/algorithm data and used by the HTTP read API to select a result |
| `city` | JSON string | Required | Required | Copied as the property/city label |
| `lat` | JSON number | Required | Required | Used by Meteo when requesting a forecast |
| `lon` | JSON number | Required | Required | Used by Meteo when requesting a forecast |
| `panel_capacitykwh` | JSON number | Required | Ignored | Stored in InputCache configuration but not used by the current algorithm |
| `panel_tiltdegrees` | JSON number | Required | Ignored | Stored in InputCache configuration but not used by the current algorithm |
| `panel_azimuthdegrees` | JSON number | Required | Ignored | Stored in InputCache configuration but not used by the current algorithm |
| `electricity_area` | JSON string | Required | Required | Associates weather data with one of the price areas used by the algorithm |

Jansson's `F` conversion accepts a JSON integer or real for the numeric fields
other than `id`. The Meteo C++ reader likewise accepts either numeric JSON type
for latitude and longitude. Neither reader currently validates coordinate
ranges, panel-value ranges, uniqueness or positivity of `id`, or membership of
`electricity_area` in `SE1` through `SE4`.

The in-memory InputCache area field has room for three characters plus a null
terminator. The Meteo property field has room for four characters plus a null
terminator. Use the exact current values `SE1`, `SE2`, `SE3`, or `SE4`; longer
strings are truncated and will not reliably match the algorithm's area names.

## Parser behavior and limits

Two independent readers consume the same file, and their failure behavior is
not identical:

- InputCache calls `homesystem_LoadAllCount` with a maximum of five. It examines
  only the first five array entries. Each entry must contain all eight fields
  with the types above. A failed entry is reported, but the function still
  returns the truncated array length rather than the number of successfully
  parsed entries. InputCache's containing structure is zero-initialized, so a
  rejected slot remains zero/empty while `home_count` can still include it.
- Meteo scans entries in order until it has accepted five valid properties or
  reaches the end. It requires only `id`, `city`, `lat`, `lon`, and
  `electricity_area`; malformed entries are silently skipped. Panel fields do
  not affect Meteo acceptance.
- `homesystem_LoadAllCount` returns `-1` for a missing file, invalid JSON, or a
  root without a `systems` array. InputCache stores that result in unsigned
  `size_t home_count` and checks only for zero, so `-1` becomes `SIZE_MAX` and
  initialization can incorrectly report success. The systemd unit's
  readability check catches a missing or unreadable installed file, and the
  deploy script checks JSON syntax, but neither substitutes for correct schema
  error propagation. An empty `systems` array returns zero and does make
  InputCache initialization fail.
- Meteo's loader returns success for an empty array or when every entry is
  skipped. `meteo::fetchAll` also treats a zero-property set as success, after
  which the oneshot writes a zero-property fixed structure and can exit
  successfully. Process success therefore does not prove that weather was
  requested for any property.

The number five appears independently in InputCache (`MAX_HOMES`), Meteo
(`PROPERTIES_MAX`), and the algorithm/shared result model (`MAX_ID`). It is a
temporary test limit. Raising it safely requires a coordinated review of every
fixed-size structure and IPC participant; editing only the JSON file or one
constant does not increase end-to-end capacity.

## Seed-data contradictions

The repository seed contains 17 objects, but the current services consume no
more than five. Entries 1 through 10 use integer IDs and contain a `city`.
Entries 11 through 17 instead use string IDs and omit `city`. Those later
entries do not satisfy either current parser's schema.

Because both current readers reach their five-property limit before those
objects, the contradiction is normally hidden. It is still unsafe to treat all
17 objects as valid examples or to reorder a string-ID/missing-city object into
the first five. The seed should eventually be reconciled with the implemented
schema or replaced when the planned identity model is designed.

There is also a second copy at
[`API/Meteo/Glennergy-Fastigheter.json`](../API/Meteo/Glennergy-Fastigheter.json).
Current production paths use the root `API/Glennergy-Fastigheter.json` as the
installation seed and `/etc/glennergy/fastigheter.json` at runtime. The nested
copy is not authoritative for the current C++ Meteo executable.

## Current identity and access model

The current property identifier is a manually assigned JSON integer. It is
passed without authentication in the read-only HTTP request path. The server
does not prove that the caller owns the requested property, and no implemented
endpoint appends to or updates `fastigheter.json`.

Treat this as temporary development behavior, not as a secure device identity
or authorization design. Do not place secrets, private keys, API keys, or
credentials in this configuration file.

## Planned registration and identity

The agreed direction is two-way communication in which an ESP32-S3 can register
property information and Glennergy persists it in the property configuration.
Each physical ESP32-S3 should ultimately be associated with a UUID-like unique
identity instead of relying on the current increasing integer alone.

That work is **planned, not implemented**. The following decisions remain open
and must not be inferred from the current seed:

- how the UUID-like value is generated or sourced;
- whether property identity and device identity are the same value or have a
  separate mapping;
- the registration request and response schema;
- authentication, authorization, enrollment, rotation, and recovery;
- duplicate registration and ownership-transfer behavior;
- migration of existing integer IDs and API compatibility;
- validation, atomic persistence, concurrency, limits, and failure recovery.

Until those questions are resolved and code exists in both repositories,
documentation and clients must continue to label registration and UUID-like
identity as planned.

## Safe maintenance checklist

Before changing the schema, property limit, or identity model:

1. Update and test both property readers, not just the seed JSON.
2. Review the fixed-size types and raw IPC layouts shared by Meteo, InputCache,
   Algorithm, and the HTTP server.
3. Define validation and error behavior for every field.
4. Update the Glennergy HTTP API reference, the Glennergy-ESP consumer
   interface contract, and current limitations.
5. Preserve an existing installed configuration during deployment and provide
   an explicit migration when its format changes.
6. Keep real production addresses and all credentials out of examples and
   repository history.

## Implementation evidence

| Claim area | Authoritative source |
|---|---|
| Installed and seed paths | [`Libs/GlennergyPaths.h`](../Libs/GlennergyPaths.h), [`Makefile`](../Makefile), [`glennergy_deploy.sh`](../glennergy_deploy.sh) |
| C schema and bounded load behavior | [`Libs/Homesystem.c`](../Libs/Homesystem.c), [`Libs/Homesystem.h`](../Libs/Homesystem.h) |
| InputCache startup and five-home limit | [`Cache/InputCache.c`](../Cache/InputCache.c), [`Cache/InputCache.h`](../Cache/InputCache.h) |
| Meteo schema, skip behavior, and limit | [`API/Meteocpp/Meteo.cpp`](../API/Meteocpp/Meteo.cpp), [`API/Meteocpp/meteo_types.hpp`](../API/Meteocpp/meteo_types.hpp) |
| Algorithm result capacity | [`Algorithm/AlgoritmProtocol.h`](../Algorithm/AlgoritmProtocol.h) |
| Current HTTP ID lookup | [`Server/Connection/Connection.c`](../Server/Connection/Connection.c) |
| Seed contradictions | [`API/Glennergy-Fastigheter.json`](../API/Glennergy-Fastigheter.json) |

Re-review this guide whenever any evidence file above changes. Verify planned
identity and registration statements with the project owner because those
requirements are intentionally unresolved beyond the direction recorded here.
