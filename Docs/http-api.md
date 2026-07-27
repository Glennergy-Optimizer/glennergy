# Glennergy HTTP API reference

> **Quick answer:** Glennergy currently provides three unauthenticated read
> endpoints—recommendation, weather and price—using a temporary route shape.

This is the complete server-side reference for the HTTP API implemented by
Glennergy. It describes the server's routes, methods, statuses, headers,
schemas, edge cases, security boundaries and planned direction.

Glennergy-ESP retains its own complete
[consumer-facing interface contract](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/interface-contract.md)
for firmware-specific parsing, cache, retry, health, UI and compatibility
details. Those consumer implementation details are intentionally excluded from
this server reference. Shared wire changes must update both documents.

This reference is a code-level implementation contract, not a promise that the
current API design is final.

## Current interaction at a glance

Clients perform read-only HTTP `GET` requests. Glennergy reads the requested
property's latest algorithm snapshot from shared memory and returns one JSON
array for recommendation, weather or price.

| Need | Current request |
| --- | --- |
| Recommendation | `GET ${LEOP_BASE_URL}/id=2?recommendation` |
| Weather | `GET ${LEOP_BASE_URL}/id=2?weather` |
| Electricity price | `GET ${LEOP_BASE_URL}/id=2?price` |

For a first read, continue through **Response status and error behavior**. The
schema and limitation sections are the detailed reference for implementation
and debugging.

Use a deployment-specific placeholder for the server address:

```text
LEOP_BASE_URL=http://leop.example.com
```

The production VPS address is intentionally not part of public documentation.
The address is not a security control; it belongs in deployment configuration.

## Transport and security limitations

The application server listens over plain HTTP on loopback. The repository
expects an external reverse proxy, but does not contain or verify its TLS,
firewall or public-access configuration. The read endpoints perform no
authentication or authorization, so a reachable caller can request data for
any accepted integer property ID.

Do not put tokens, passwords, private keys, GitHub Actions secrets or Wi-Fi
credentials in a URL, payload example, log, screenshot or this document. In
particular, `OPENAI_API_KEY` values and SSH private keys must never be retrieved,
used or disclosed for documentation work.

## Current request grammar

The server accepts a request target only in this exact form:

```text
/id=<non-negative-decimal-integer>?<command>
```

`<command>` must be exactly `recommendation`, `weather` or `price`, with no
additional query parameters or trailing characters. The parsed ID must fit in a
C `int`. The API is case-sensitive.

The server header parser recognizes `GET` and `OPTIONS`, but there is no general
OPTIONS handler: `/` and `/favicon.ico` return `204`, while other OPTIONS
targets continue through the same route parser. This is not a dependable CORS
preflight contract.

## Response status and error behavior

| Condition | Observed server behavior |
| --- | --- |
| Valid route and matching property ID | `200 OK`, JSON array containing 96 objects |
| Valid route but no matching positive property ID | Normally `200 OK`, empty JSON array `[]` |
| Invalid target, command, ID or supported-method parsing failure | `400 Bad Request`, empty body |
| `/` or `/favicon.ico` | `204 No Content`, empty body |
| Shared-memory/semaphore failure or oversized generated response | Handler fails; no stable JSON error response is defined |
| Incomplete request headers | Server intends a 3-second read timeout; this is not a response-status contract |

Successful data responses declare `Content-Type: application/json`, allow any
CORS origin, and close the connection. Error responses are empty rather than a
structured JSON error schema. The server does not use `404` for an unknown
property ID.

There is no negotiated API version or structured error schema.

Property ID `0` is an unsafe edge case rather than a valid selector. The parser
accepts it, and zero-initialized unused shared-memory result slots also have ID
zero. A request for ID zero may therefore match unused slots and serialize
zero/empty datasets instead of returning `[]`.

## Current JSON schemas

All three successful responses are top-level JSON arrays. Glennergy emits
exactly 96 entries for a matching property because its served result arrays are
fixed to 96 quarter-hour slots.

### Recommendation

```json
[
  {
    "id": 2,
    "type": 0.42,
    "timestamp": "2026-01-01T12:00",
    "temp": 18.5
  }
]
```

| Field | JSON type | Server source/meaning |
| --- | --- | --- |
| `id` | Integer | Property ID from the matching algorithm result |
| `type` | Real | Current numeric value from `AlgoritmResult.recommendation[]`; intended semantics unresolved |
| `timestamp` | String | Timestamp copied from the matched spot-price sample |
| `temp` | Real | Matched weather temperature in degrees Celsius |

The intended meaning of `recommendation[].type` is **unresolved**. The algorithm
calculates a categorical recommendation value but currently discards it; the
published array is instead assigned the result of a price-position calculation.
This document deliberately does not name `type` as buy, hold, sell, percentage
or another final semantic.

### Weather

```json
[
  {
    "timestamp": "2026-01-01T12:00",
    "temp": 18.5,
    "weather_code": 3,
    "uv_index": 1.0
  }
]
```

| Field | JSON type | Unit/meaning |
| --- | --- | --- |
| `timestamp` | String | Matched quarter-hour start |
| `temp` | Real | Degrees Celsius |
| `weather_code` | Integer | Open-Meteo/WMO weather code |
| `uv_index` | Real | UV index, emitted from a server-side integer value |

The source pipeline converts upstream UV values to an integer before the
response is generated, losing fractions, and then serializes that integer value
as a JSON real.

### Electricity price

```json
[
  {
    "timestamp": "2026-01-01T12:00",
    "price SEK": 0.73
  }
]
```

| Field | JSON type | Unit/meaning |
| --- | --- | --- |
| `timestamp` | String | Quarter-hour price interval start |
| `price SEK` | Real | SEK per kWh |

The space in `price SEK` is part of the current wire key and must be preserved
for compatibility.

### Timestamp format

Glennergy stores source timestamps in 32-byte buffers and returns them without
normalizing or declaring a single wire format. A value can include a UTC offset,
for example `2026-06-10T14:45:00+02:00`. A future contract should specify the
format, offset policy and validation.

## Known API limitations

| Area | Current server limitation or risk |
| --- | --- |
| Recommendation semantics | `type` has no approved final meaning; a calculated categorical value is discarded before publication. |
| Timestamp format | No single normalized wire format or offset policy is declared. |
| UV precision/type | Upstream UV data is converted to an integer and then emitted as a JSON real. |
| Versioning | No API or schema version is negotiated or returned. |
| Error schema | Error responses are empty and do not provide a stable structured body. |
| Unknown property | A missing positive property ID returns `200 []`, not `404`. |
| Property ID zero | Parser accepts zero, which can match zero-initialized unused server result slots and produce misleading zero/empty datasets. |
| Partial/zero-filled data | The server serializes all 96 slots, even if an algorithm slot was not populated with meaningful current data. |
| Freshness | Responses contain no version, age, `ETag` or server-generated freshness metadata. |
| Access control | Read routes are unauthenticated and authorize no property ownership. |

## Planned API correction (not implemented)

The accepted direction is to replace the backwards request grammar with a
resource-first form such as:

```text
GET ${LEOP_BASE_URL}/recommendation?id=2
GET ${LEOP_BASE_URL}/weather?id=2
GET ${LEOP_BASE_URL}/price?id=2
```

These paths do not work in the verified `dev` implementations. The final route
names, compatibility window, versioning and migration sequence must be agreed
and implemented in both repositories before this section can become current.

## Planned identity and registration (not implemented)

The intended direction is a state-changing registration API through which a
device can submit property information for safe persistence. Device identity
should use a UUID or similarly unique identifier rather than relying only on
the current increasing integer property IDs.

No registration route, HTTP method, payload schema, UUID source, property-ID
mapping, authentication mechanism, authorization policy, duplicate/update
behavior, validation rules, response schema, retry/idempotency contract or safe
persistence design has been approved or implemented. None is invented here.
State-changing registration must not be exposed publicly until those decisions,
transport protection and credential lifecycle are designed and tested.

The current temporary server capacity of five properties is a test limit, not
a final product limit or wire-contract guarantee.

## Stable-production comparison

The stable Glennergy `origin/main` reference recorded for comparison was
`61761b5eda30bee417a0b6e33e10fb061e18db26`. It contains an older
request/response path and does not implement the current `dev` server's
validated three-command grammar and weather/price branches. This document
describes the current `dev` server API; stable-production behavior requires
separate deployment/runtime verification.

## Compatibility and maintenance requirements

Treat any change to the following as an HTTP API contract change:

- base-URL configuration or HTTP/HTTPS behavior;
- route grammar, method, status or error body;
- property/device identity rules;
- field name, JSON type, unit, meaning or timestamp format;
- array length, ordering or empty-result behavior;
- registration, authentication or authorization.

For each such change:

1. Update the server implementation and this reference; notify known consumers
   and explicitly document the compatibility window.
2. Add or update server parser, serialization and route tests.
3. Test valid, empty, malformed, oversized and unknown-property responses.
4. Verify all three payloads against controlled fixtures without contacting
   production unless separately authorized.
5. Update this server reference and the Glennergy-ESP consumer contract,
   including their SHAs, compatibility tables and planned/current labels.

## Verification evidence

This edition describes Glennergy `dev` source snapshot
`42798bee227fcd621cbcb0b37c2b5da771210086` and was last reviewed on
2026-07-26. Stable-production differences are described above.

Primary producer evidence in Glennergy:

- `Server/HTTP/HTTPRequest.c` — exact route and command parser
- `Server/Connection/Connection.c` — statuses, response headers and JSON fields
- `Algorithm/AlgoritmProtocol.h` — five-result/96-slot shared-memory limits
- `Algorithm/main.c` — recommendation assignment, field population and timestamp source
- `API/Meteo/Meteo.h` and `API/Spotpris/Spotpris.h` — units and source types

This contract was verified statically against source and branch history. It was
not validated against the production VPS or a running local server.
