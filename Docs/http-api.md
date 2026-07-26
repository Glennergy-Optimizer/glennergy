# Glennergy HTTP API reference

| Metadata | Value |
| --- | --- |
| Status | Current implementation reference, with planned changes explicitly separated |
| Audience | API consumers, Glennergy and Glennergy-ESP developers, and maintainers |
| Canonical owner | Glennergy for server routes, methods, statuses, headers and response schemas |
| Applies to | Authoritative `dev`; stable-production differences are noted below |
| Last verified | 2026-07-26 |
| Glennergy-ESP `dev` | `b5a502afd9ca2ae374b3131b0031b8390f93b348` |
| Glennergy `dev` | `42798bee227fcd621cbcb0b37c2b5da771210086` |

This is the complete server-side reference for the HTTP API implemented by
Glennergy. It describes the producer's routes, methods, statuses, headers,
schemas, edge cases, security boundaries and planned direction. It also records
the current Glennergy-ESP consumer behavior where that behavior changes how an
API response is interpreted safely.

Glennergy-ESP retains its own complete
[consumer-facing interface contract](https://github.com/Glennergy-Optimizer/Glennergy-ESP/blob/dev/docs/interface-contract.md)
covering every current endpoint and behavior relevant to the firmware, plus
parser, cache, retry and UI integration consequences. Server API changes must
update both documents together.

This reference is a code-level implementation contract, not a promise that the
current API design is final.

## Current interaction at a glance

Glennergy-ESP currently performs read-only HTTP `GET` requests. Glennergy reads
the requested property's latest algorithm snapshot from shared memory and
returns one JSON array. The ESP fetches recommendation, weather and price as
three separate requests and caches each raw response in SPIFFS.

Use a deployment-specific placeholder for the server address:

```text
LEOP_BASE_URL=http://leop.example.com
```

The production VPS address is intentionally not part of public documentation.
The address is not a security control; it belongs in deployment configuration.

## Transport and security limitations

The current ESP implementation uses plain HTTP. It configures no server
certificate and sends no API credential, device credential or authorization
header. The server's read endpoints perform no authentication or authorization.
Traffic can therefore be observed or modified by a network intermediary, and a
reachable caller can request data for any accepted integer property ID.

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

Glennergy-ESP currently hard-codes the temporary example property ID `2` and
sends these requests:

| Data | Method and current request |
| --- | --- |
| Recommendation | `GET ${LEOP_BASE_URL}/id=2?recommendation` |
| Weather | `GET ${LEOP_BASE_URL}/id=2?weather` |
| Electricity price | `GET ${LEOP_BASE_URL}/id=2?price` |

Only `GET` is used by the ESP. The server header parser recognizes `GET` and
`OPTIONS`, but there is no general OPTIONS handler: `/` and `/favicon.ico`
return `204`, while other OPTIONS targets continue through the same route
parser. This is not a dependable CORS preflight contract.

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

The ESP's normal fetch path does **not** require a 2xx status before accepting a
response body: `HTTPClient_GET` records the status, but the three data modules
only test whether a body exists and then attempt to cache and parse it. The
parsers reject invalid top-level JSON, non-arrays and empty arrays. They do not
safely validate every object member. Missing or wrong numeric member types can
silently become zero through Jansson accessors, while a missing or non-string
timestamp can pass a null pointer to `strncpy`. There is no negotiated API
version.

Property ID `0` is an unsafe edge case rather than a valid selector. The parser
accepts it, and zero-initialized unused shared-memory result slots also have ID
zero. A request for ID zero may therefore match unused slots and serialize
zero/empty datasets instead of returning `[]`.

## Current JSON schemas

All three successful responses are top-level JSON arrays. Glennergy emits
exactly 96 entries for a matching property because its served result arrays are
fixed to 96 quarter-hour slots. The ESP storage is also fixed to 96 entries.

> Safety limitation: the ESP parsers do not check that an incoming array has at
> most 96 entries before copying it. A server response larger than 96 entries
> can write beyond the destination arrays. Producers must not exceed 96 until
> bounds checking is implemented.

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

| Field | Server output | ESP consumption |
| --- | --- | --- |
| `id` | JSON integer; property ID | Read with `json_integer_value`; missing/wrong type is not rejected and can become zero |
| `type` | JSON real from `AlgoritmResult.recommendation[]` | Read with `json_real_value`; missing/wrong type is not rejected and can become zero |
| `timestamp` | JSON string copied from the matched spot-price timestamp | Copied into a 20-byte buffer without a safe member/type check |
| `temp` | JSON real, degrees Celsius | Emitted but ignored by the recommendation parser |

The intended meaning of `recommendation[].type` is **unresolved**. The algorithm
calculates a categorical recommendation value but currently discards it; the
published array is instead assigned the result of a price-position calculation.
This document deliberately does not name `type` as buy, hold, sell, percentage
or another final semantic. Treat consumers that interpret it more specifically
as depending on unconfirmed behavior.

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

| Field | Server output | Unit/meaning | ESP consumption |
| --- | --- | --- | --- |
| `timestamp` | JSON string | Matched quarter-hour start | Copied into a 20-byte buffer without a safe member/type check |
| `temp` | JSON real | Degrees Celsius | Read as `double`; missing/wrong type is not rejected |
| `weather_code` | JSON integer | Open-Meteo/WMO weather code | Read as `int`; missing/wrong type is not rejected |
| `uv_index` | JSON real, sourced from a server-side integer | UV index | ESP requests a JSON integer and stores `int` |

The `uv_index` JSON-type mismatch is significant: Glennergy constructs a JSON
real while Glennergy-ESP calls `json_integer_value`. With Jansson, a non-integer
JSON value does not satisfy that accessor, so the current consumer can record
zero rather than the emitted value. The source pipeline also converts upstream
UV values to an integer before the response is generated, losing fractions.

### Electricity price

```json
[
  {
    "timestamp": "2026-01-01T12:00",
    "price SEK": 0.73
  }
]
```

| Field | Server output | Unit/meaning | ESP consumption |
| --- | --- | --- | --- |
| `timestamp` | JSON string | Quarter-hour price interval start | Copied into a 20-byte buffer without a safe member/type check |
| `price SEK` | JSON real | SEK per kWh | Read as `double`; missing/wrong type is not rejected |

The space in `price SEK` is part of the current wire key and must be preserved
for compatibility.

### Timestamp compatibility

Glennergy stores source timestamps in 32-byte buffers and returns them without
normalizing or declaring a single wire format. The ESP copies at most 19
characters plus a null terminator. A value such as
`2026-06-10T14:45:00+02:00` is therefore truncated to
`2026-06-10T14:45:00`, losing its UTC offset. Consumers must currently treat the
ESP timestamp as a display/source string rather than a reliably timezone-aware
instant. A future contract should specify format, offset policy and validation.

## ESP fetch, cache and connectivity behavior

- Each normal HTTP request has a 5,000 ms client timeout.
- The fetch task requests recommendation, then weather, then price
  synchronously. It does not retry an individual request within that fetch.
- The next full fetch is scheduled from the configured interval in minutes;
  invalid or absent interval state falls back to one minute.
- If all three parses succeed, connectivity becomes `CONNECTED`. If only some
  succeed, it becomes `DEGRADED`. If none succeed, a consecutive-failure
  counter increases.
- After three consecutive total failures, connectivity becomes `UNAVAILABLE`.
- A failed/unavailable health cycle is retried after 10 seconds. Healthy or
  partially successful fetches schedule the next health check after 60 seconds.
- The health check is not a dedicated endpoint. It performs a `GET` against the
  recommendation URL with a 5,000 ms timeout and considers only 2xx status a
  successful probe.
- When Wi-Fi is unavailable, the ESP loads each cache once for that offline
  period and publishes the cached snapshots to one-element latest-value queues.
  It tries the cache again after Wi-Fi has returned and a later offline period
  begins.
- A received body is written to `Recommendations.json`, `Weather.json` or
  `Price.json` **before** schema parsing succeeds. Consequently, a malformed or
  incompatible body can replace a previously useful cache.
- Fetch success reflects presence of a parseable, non-empty array, not freshness
  of its timestamps. No `ETag`, version, age or server-generated freshness field
  exists.

## Known producer/consumer incompatibilities

| Area | Current incompatibility or risk |
| --- | --- |
| Recommendation semantics | `type` has no approved final meaning; a calculated categorical value is discarded before publication. |
| Timestamp length | Server can emit strings longer than the ESP's 19-character payload capacity; timezone offsets can be truncated. |
| UV index | Server emits a JSON real after an earlier integer conversion; ESP reads only a JSON integer. |
| Array bounds | Server currently emits 96, but ESP parsers trust any array length despite fixed 96-entry storage. |
| HTTP status | Data fetchers do not enforce 2xx before caching/parsing the body. |
| Cache integrity | Raw bodies are cached before validation. |
| Unknown property | Server returns `200 []`; ESP treats the empty array as a parse failure, without a distinct not-found reason. |
| Property ID zero | Parser accepts zero, which can match zero-initialized unused server result slots and produce misleading zero/empty datasets. |
| Object member validation | ESP validates top-level JSON/array shape but not every member; numeric errors can become zero and invalid timestamps can be unsafe. |
| Partial/zero-filled data | The server serializes all 96 slots, even if an algorithm slot was not populated with meaningful current data. |
| Identity | Current integer property ID `2` is a temporary example and is not authenticated or tied to a device identity. |

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

The intended direction is two-way communication in which an ESP32-S3 can
register property information with Glennergy and Glennergy can persist it in
the property configuration. Device identity should be tied to a UUID or a
similarly unique identifier for each physical ESP32-S3 rather than relying only
on the current increasing integer property IDs.

No registration route, HTTP method, payload schema, UUID source, property-ID
mapping, authentication mechanism, authorization policy, duplicate/update
behavior, validation rules, response schema, retry/idempotency contract or safe
persistence design has been approved or implemented. None is invented here.
State-changing registration must not be exposed publicly until those decisions,
transport protection and credential lifecycle are designed and tested.

The current temporary server capacity of five properties is a test limit, not
a final product limit or wire-contract guarantee.

## Stable-production comparison

The stable remote branches recorded for comparison were:

- Glennergy-ESP `origin/main`: `daf35c538d84586576f8286c2d543eb1c3c89e6a`
- Glennergy `origin/main`: `61761b5eda30bee417a0b6e33e10fb061e18db26`

The stable Glennergy-ESP branch contains the three current hard-coded request
paths and parser modules. The stable Glennergy server branch contains an older
request/response path and does not implement the current `dev` server's
validated three-command grammar and weather/price branches. Static branch
comparison therefore does not prove that the two stable branches provide the
complete contract documented here. This document describes the coordinated
current `dev` integration target; stable-production behavior requires separate
deployment/runtime verification.

## Compatibility and maintenance requirements

Treat any change to the following as a cross-repository interface change:

- base-URL configuration or HTTP/HTTPS behavior;
- route grammar, method, status or error body;
- property/device identity rules;
- field name, JSON type, unit, meaning or timestamp format;
- array length, ordering or empty-result behavior;
- ESP parsing, timeout, retry, health or cache behavior;
- registration, authentication or authorization.

For each such change:

1. Update producer and consumer in a coordinated branch or explicitly document
   the compatibility window.
2. Add or update server serialization/route tests and ESP parser/fetch tests.
3. Test valid, empty, malformed, oversized and unknown-property responses.
4. Verify all three payloads end to end without contacting production or real
   hardware unless separately authorized.
5. Update this server reference and the Glennergy-ESP consumer contract,
   including their SHAs, compatibility tables and planned/current labels.

## Verification evidence

Primary producer evidence in Glennergy:

- `Server/HTTP/HTTPRequest.c` — exact route and command parser
- `Server/Connection/Connection.c` — statuses, response headers and JSON fields
- `Algorithm/AlgoritmProtocol.h` — five-result/96-slot shared-memory limits
- `Algorithm/main.c` — recommendation assignment, field population and timestamp source
- `API/Meteo/Meteo.h` and `API/Spotpris/Spotpris.h` — units and source types

Primary consumer evidence in Glennergy-ESP:

- `main/LEOP/LEOP_Fetcher.c` — current URLs, scheduling, state, retry and cache flow
- `main/HTTP.c` — GET/probe methods, status handling and 5-second timeout
- `main/JSONParser/DataParser.c` — required fields, JSON accessors and missing bounds checks
- `main/LEOP/Recommendation.c`, `Weather.c` and `Price.c` — cache-before-parse behavior
- `main/LEOP/Recommendation.h`, `Weather.h` and `Price.h` — fixed storage and field types

This contract was verified statically against source and branch history. It was
not validated against the production VPS, a running local server or physical
ESP32-S3 hardware.
