# Day 6 — JSON: Schema, Parsing, Serialization

> **Time budget:** 60 min concept · 120 min lab · 45 min drills · 15 min recap
> **Prerequisites:** Comfortable with C++17 and Python. Have `nlohmann/json` available (or installable via apt/brew).
> **By the end you can:**
> - Explain JSON spec gotchas that catch experienced engineers (integer precision, no comments, NaN)
> - Design a versioned JSON schema with `$ref`, `oneOf`, and `additionalProperties`
> - Choose between `nlohmann/json`, `simdjson`, and `RapidJSON` with concrete justification
> - Explain when to reach for Protobuf, FlatBuffers, or MessagePack over JSON
> - Write a C++ parser that handles untrusted input without crashing or leaking

---

## 1. Conceptual overview

JSON is ubiquitous in embedded systems work precisely where you don't expect it: config files loaded at startup, telemetry streamed over serial links, test harness payloads, and REST APIs to device management backends. The interview bar isn't "can you parse it"—it's "do you understand the trade-offs and failure modes."

Three areas matter:

1. **The spec** — what JSON does and doesn't guarantee (several things will surprise you)
2. **Schema design** — versioning, validation at system boundaries, and the difference between `oneOf` and `anyOf`
3. **Parser selection** — ergonomics vs throughput vs footprint, and when to abandon JSON entirely

---

## 2. Deep dive: JSON spec gotchas

### What's NOT in JSON

These all cause bugs or failed parses:

```jsonc
// ❌ Comments are not valid JSON
{ "rate": 100 }  // Hz

// ❌ Trailing commas
{ "a": 1, "b": 2, }

// ❌ NaN and Infinity — not JSON values
{ "ratio": NaN, "gain": Infinity }

// ❌ Single quotes
{ 'key': 'value' }
```

`JSON5` and `HJSON` support comments and trailing commas — know they exist but distinguish them from RFC 8259 JSON.

### Integer precision

JavaScript (and many parsers) represents numbers as IEEE 754 doubles. Integers larger than 2^53 (9,007,199,254,740,992) lose precision.

```json
{ "device_id": 9007199254740993 }
```

A JS-based parser may silently change this to `9007199254740992`. Fix: use strings for large integer IDs, or `int64` fields in typed schemas.

```json
{ "device_id": "9007199254740993" }
```

### Duplicate keys

RFC 8259 says parsers "SHOULD" treat duplicate keys as an error but don't have to. In practice, different parsers keep the first, keep the last, or reject. Don't rely on duplicate key behavior.

### Unicode and escaping

JSON strings are Unicode (UTF-8 encoding of the file). `\uXXXX` escape sequences are BMP code points. Supplementary characters (emoji, some CJK) require surrogate pairs (`\uD83D\uDE00`). Most modern parsers handle this transparently, but it matters when generating JSON in C with manual string building.

---

## 3. Deep dive: JSON Schema

### Core vocabulary

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://example.com/telemetry.schema.json",
  "type": "object",
  "required": ["device_id", "timestamp", "readings"],
  "additionalProperties": false,
  "properties": {
    "device_id": {
      "type": "string",
      "format": "uuid"
    },
    "timestamp": {
      "type": "integer",
      "description": "Unix epoch milliseconds",
      "minimum": 0
    },
    "readings": {
      "type": "array",
      "minItems": 1,
      "items": { "$ref": "#/$defs/Reading" }
    },
    "tags": {
      "type": "object",
      "additionalProperties": { "type": "string" }
    }
  },
  "$defs": {
    "Reading": {
      "type": "object",
      "required": ["name", "value", "unit"],
      "additionalProperties": false,
      "properties": {
        "name":  { "type": "string", "minLength": 1 },
        "value": { "type": "number" },
        "unit":  { "type": "string", "enum": ["C", "Pa", "m/s", "V", "A"] }
      }
    }
  }
}
```

### `oneOf` vs `anyOf` vs `allOf`

| Keyword | Meaning | Use when |
|---|---|---|
| `oneOf` | Exactly one subschema must match | Discriminated union — only one type is valid |
| `anyOf` | At least one must match | Union where overlap is acceptable |
| `allOf` | All must match | Extend/compose schemas |

**`oneOf` gotcha:** validation checks all branches, so it's slower than a discriminator field. For embedded telemetry with many message types, prefer a `type` field + `if/then/else` or a `$ref` per type keyed on that field.

### Versioning strategy

Common pattern — version in the schema `$id` and in the message:

```json
{
  "schema_version": 2,
  "device_id": "...",
  "timestamp": 1700000000000,
  "readings": [...]
}
```

Schema evolution rules that don't break old clients:
- Adding optional fields (`required` not updated) — backwards compatible
- Adding to `enum` values — backwards compatible for consumers that ignore unknowns
- Removing required fields — **breaking**
- Changing a field's type — **breaking**
- Setting `additionalProperties: false` on a previously open schema — **breaking**

For long-lived APIs, keep `additionalProperties` absent (open) in the base schema, validate known fields explicitly, and ignore unknowns.

---

## 4. Deep dive: C++ JSON parsers

### Comparison table

| | `nlohmann/json` | `simdjson` | `RapidJSON` |
|---|---|---|---|
| **Ergonomics** | Excellent — STL-like API | Good — but streaming API differs | Verbose, SAX/DOM |
| **Parse speed** | ~300 MB/s | ~2–3 GB/s (SIMD) | ~500 MB/s |
| **Memory footprint** | Medium (DOM in memory) | Low (on-demand) | Low (in-place optional) |
| **Header-only** | Yes | No | Yes |
| **C++ standard** | C++11 minimum | C++17 | C++11 |
| **Choose when** | Readability matters; test tooling | High-throughput parsing; CI pipelines | Embedded, low memory |

`simdjson` uses SIMD instructions (SSE4.2 / AVX2 / NEON) to parse at memory bandwidth. The trade-off: it requires the input in a contiguous buffer with a few bytes of padding. It doesn't support streaming from a file descriptor — you must read the whole document first.

### nlohmann/json: safe parsing of untrusted input

```cpp
// telemetry.cpp
// Build: g++ -std=c++17 -Wall -Wextra -o telemetry telemetry.cpp
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>

using json = nlohmann::json;

struct Reading {
    std::string name;
    double value;
    std::string unit;
};

struct TelemetryMsg {
    std::string device_id;
    int64_t timestamp_ms;
    std::vector<Reading> readings;
};

// Returns std::nullopt on any parse or validation failure.
// Never throws into the caller — parse errors are expected input, not bugs.
std::optional<TelemetryMsg> parse_telemetry(std::string_view raw) {
    json j;
    try {
        j = json::parse(raw);
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return std::nullopt;
    }

    // Validate required fields — .at() throws json::out_of_range if missing
    // .get<T>() throws json::type_error if wrong type
    try {
        TelemetryMsg msg;
        msg.device_id    = j.at("device_id").get<std::string>();
        msg.timestamp_ms = j.at("timestamp").get<int64_t>();

        for (const auto& r : j.at("readings")) {
            Reading rd;
            rd.name  = r.at("name").get<std::string>();
            rd.value = r.at("value").get<double>();
            rd.unit  = r.at("unit").get<std::string>();
            msg.readings.push_back(std::move(rd));
        }
        return msg;

    } catch (const json::exception& e) {
        std::cerr << "Validation error: " << e.what() << "\n";
        return std::nullopt;
    }
}

TelemetryMsg make_sample() {
    return {
        .device_id    = "a1b2c3d4-0000-0000-0000-000000000001",
        .timestamp_ms = 1700000000000LL,
        .readings     = {{"temp", 23.5, "C"}, {"pressure", 101325.0, "Pa"}},
    };
}

json serialize(const TelemetryMsg& msg) {
    json j;
    j["device_id"] = msg.device_id;
    j["timestamp"] = msg.timestamp_ms;
    j["readings"]  = json::array();
    for (const auto& r : msg.readings) {
        j["readings"].push_back({{"name", r.name}, {"value", r.value}, {"unit", r.unit}});
    }
    return j;
}

int main() {
    // Round-trip test
    auto original = make_sample();
    std::string serialized = serialize(original).dump();
    std::cout << "Serialized: " << serialized << "\n";

    auto parsed = parse_telemetry(serialized);
    if (!parsed) { std::cerr << "Round-trip failed\n"; return 1; }

    std::cout << "device_id: " << parsed->device_id << "\n";
    std::cout << "readings:  " << parsed->readings.size() << "\n";

    // Test with bad input
    auto bad = parse_telemetry(R"({"device_id": 42})");  // wrong type
    if (!bad) std::cout << "Correctly rejected bad input\n";

    return 0;
}
```

```bash
# Install nlohmann/json (header-only)
sudo apt-get install -y nlohmann-json3-dev 2>/dev/null || \
  brew install nlohmann-json

g++ -std=c++17 -Wall -Wextra -o telemetry telemetry.cpp
./telemetry
```

---

## 5. Deep dive: alternatives to JSON

This question comes up in every embedded interview. Have a crisp answer.

| Format | Binary | Schema | Self-describing | Streaming | Choose when |
|---|---|---|---|---|---|
| JSON | No | Optional | Yes | With care | Config, REST APIs, debugging |
| Protocol Buffers | Yes | Required | No | Yes | High-throughput, schema evolution, cross-language |
| FlatBuffers | Yes | Required | No | Yes | Zero-copy reads; embedded with tight memory budget |
| MessagePack | Yes | Optional | Yes | Yes | Drop-in JSON replacement where binary matters |
| CBOR | Yes | Optional | Yes | Yes | IoT (used in IETF standards, CoAP) |

**Key differences to know:**

- **FlatBuffers vs Protobuf:** FlatBuffers don't require a parse step — you read fields directly from the buffer. No allocation on read. Used in game engines and high-frequency trading. Protobuf is more ergonomic and has broader tooling.

- **MessagePack:** Structurally identical to JSON but binary-encoded. Easy to swap in if you already have JSON tooling. 20–50% smaller, 2–4x faster to parse.

- **"When NOT to use JSON?"** — binary protocol over serial (framing + size matter), >10 MB messages parsed at high rate, schema must be enforced by the format (not just by convention), or message size is constrained (sensor over LoRa: 242-byte max payload).

---

## 6. Lab

### Setup

```bash
mkdir -p day6 && cd day6
sudo apt-get install -y nlohmann-json3-dev python3-jsonschema 2>/dev/null
pip3 install jsonschema pydantic --break-system-packages 2>/dev/null || \
  pip install jsonschema pydantic
```

### Tasks

**Task 1: Design the telemetry JSON schema**

Save the schema from Section 3 as `telemetry.schema.json`. Add one additional constraint: `device_id` must match the UUID regex `^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$`.

Done when: a compliant message validates, a message with missing `timestamp` fails, and a message with an invalid unit fails.

**Task 2: Python validator**

```python
# validate_telemetry.py
import json
import sys
from pathlib import Path

import jsonschema

SCHEMA = json.loads(Path("telemetry.schema.json").read_text())

VALID_SAMPLE = {
    "device_id": "a1b2c3d4-0000-0000-0000-000000000001",
    "timestamp": 1700000000000,
    "readings": [
        {"name": "temp", "value": 23.5, "unit": "C"},
        {"name": "pressure", "value": 101325.0, "unit": "Pa"},
    ],
}

INVALID_SAMPLES = [
    ({}, "empty object"),
    ({"device_id": "x", "timestamp": 0, "readings": []}, "empty readings"),
    ({"device_id": "not-a-uuid", "timestamp": 0,
      "readings": [{"name": "t", "value": 1.0, "unit": "C"}]}, "bad uuid"),
    ({"device_id": "a1b2c3d4-0000-0000-0000-000000000001", "timestamp": -1,
      "readings": [{"name": "t", "value": 1.0, "unit": "C"}]}, "negative timestamp"),
]

def validate(msg: dict) -> list[str]:
    validator = jsonschema.Draft202012Validator(SCHEMA)
    return [e.message for e in validator.iter_errors(msg)]


if __name__ == "__main__":
    errors = validate(VALID_SAMPLE)
    assert not errors, f"Valid sample failed: {errors}"
    print("✓ Valid sample passes")

    for sample, label in INVALID_SAMPLES:
        errors = validate(sample)
        assert errors, f"Invalid sample '{label}' should have failed"
        print(f"✓ '{label}' correctly rejected: {errors[0][:60]}...")

    print("\nAll assertions passed.")
```

```bash
python3 validate_telemetry.py
```

Done when: all assertions pass.

**Task 3: C++ producer + consumer over FIFO**

Take the C++ code from Section 4 and extend `main()` to write 10 serialized messages to the FIFO from Day 1, then have the consumer validate and print each.

Done when: you can pipe the producer output to the consumer and see 10 parsed messages, or an error message for deliberately malformed input.

**Task 4: Benchmark nlohmann vs simdjson (optional — do if time allows)**

```cpp
// bench.cpp
// Build: g++ -std=c++17 -O2 -o bench bench.cpp -lsimdjson
#include <nlohmann/json.hpp>
#include <simdjson.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

// Generate test data: 10k telemetry messages
std::string generate_corpus(int n) {
    nlohmann::json arr = nlohmann::json::array();
    for (int i = 0; i < n; ++i) {
        arr.push_back({
            {"device_id", "a1b2c3d4-0000-0000-0000-000000000001"},
            {"timestamp", 1700000000000LL + i},
            {"readings", {{{"name","temp"},{"value",23.5},{"unit","C"}}}},
        });
    }
    return arr.dump();
}

int main() {
    const std::string corpus = generate_corpus(10000);
    const int ITERS = 20;

    // nlohmann benchmark
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        auto j = nlohmann::json::parse(corpus);
        (void)j.size();
    }
    auto t1 = std::chrono::steady_clock::now();
    double nlohmann_ms = std::chrono::duration<double, std::milli>(t1 - t0).count() / ITERS;

    // simdjson benchmark
    simdjson::ondemand::parser parser;
    // simdjson needs padded input
    simdjson::padded_string padded(corpus);

    auto t2 = std::chrono::steady_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        auto doc = parser.iterate(padded);
        auto arr = doc.get_array();
        size_t count = 0;
        for (auto elem : arr) { ++count; (void)elem; }
        (void)count;
    }
    auto t3 = std::chrono::steady_clock::now();
    double simdjson_ms = std::chrono::duration<double, std::milli>(t3 - t2).count() / ITERS;

    size_t bytes = corpus.size();
    std::cout << "Corpus: " << bytes / 1024 << " KB\n";
    std::cout << "nlohmann: " << nlohmann_ms << " ms  ("
              << bytes / nlohmann_ms / 1e3 << " MB/s)\n";
    std::cout << "simdjson: " << simdjson_ms << " ms  ("
              << bytes / simdjson_ms / 1e3 << " MB/s)\n";
}
```

---

## 7. Common pitfalls

- **`additionalProperties: false` on a schema you intend to evolve.** Old clients will reject new messages that add fields. Leave it open or use a versioning strategy.

- **`j["key"]` in nlohmann on a missing key inserts a null.** Use `j.at("key")` when you want a `json::out_of_range` exception instead of silent null insertion.

- **Assuming `int` is sufficient for timestamps.** `int` is 32 bits on most platforms. Unix millisecond timestamps need `int64_t`. Use `j.get<int64_t>()` explicitly.

- **Validating JSON only at the application layer.** Validate at every trust boundary — network input, file load, IPC input. Never inside tight loops where you control the data.

- **Ignoring parse errors and proceeding.** An unparsed `json::exception` from a library call can propagate as `std::terminate`. Always wrap in `try/catch`.

- **`oneOf` performance with many variants.** Every alternative is checked, even after a match. For 20+ message types, use a discriminator field approach: `if/then/else` with a `const` on the type field.

---

## 8. Interview drills

**Q: When would you NOT use JSON?**

Binary embedded protocols (serial, CAN, UART) where every byte counts. High-throughput pipelines where you're parsing millions of messages per second and memory allocation matters — use FlatBuffers or simdjson with on-demand parsing. Anywhere schema evolution is enforced by the format (Protobuf's unknown field handling is more robust than JSON's `additionalProperties`). For inter-device protocols, consider CBOR (binary JSON superset, used in CoAP/IETF IoT standards).

*Follow-up: "Would you use JSON for a config file read once at startup?" Yes — readability and editability outweigh parse cost at startup.*

---

**Q: How do you evolve a JSON schema without breaking old clients?**

Add fields as optional (don't add to `required`). Never remove required fields without a version bump. Never change a field's type. Keep `additionalProperties` absent or `true` in the base schema. Add a `schema_version` field and branch on it in the parser. For hard breaking changes, bump the version and run two parsers in parallel during the transition window.

*Follow-up: "What if you need to rename a field?" Support both the old and new name during a transition period; deprecate the old one, remove it in v+2.*

---

**Q: What's the difference between `oneOf` and `anyOf`?**

`oneOf`: exactly one subschema must match — overlapping schemas cause validation failure. `anyOf`: one or more can match — overlap is fine. In practice, `oneOf` is for discriminated unions (a message is either type A or type B, never both). `anyOf` is for "the value satisfies at least one of these constraints" which is common for nullable fields (`anyOf: [{type: string}, {type: null}]`). In JSON Schema 2020-12, use `type: ["string", "null"]` for nullable, not `anyOf`.

---

**Q: Walk me through error handling when parsing untrusted JSON in C++.**

Wrap `json::parse` in a try/catch for `json::parse_error` (malformed JSON). Wrap field accesses using `.at()` in a catch for `json::out_of_range` (missing key) and `json::type_error` (wrong type). Return `std::optional<T>` or `std::expected<T, Error>` from the parse function — never throw into the caller's business logic. Log the raw error with position information for debugging. Never crash on malformed network input.

---

## 9. Cheatsheet

### JSON spec: what's missing

| Feature | Valid? | Fix |
|---|---|---|
| Comments | No | Use JSON5 or strip pre-parse |
| Trailing commas | No | Lint before parse |
| NaN / Infinity | No | Use null or string sentinel |
| Int > 2^53 | Lossy in JS | Use string for large IDs |
| Duplicate keys | Undefined | Don't emit them |

### JSON Schema keywords

```
type            string | number | integer | boolean | array | object | null
required        ["field1", "field2"]
properties      { "field": <schema> }
additionalProperties  false | <schema>
items           <schema>   (array element schema)
$ref            "#/$defs/TypeName"
oneOf / anyOf / allOf   [<schema>, ...]
enum            ["A", "B", "C"]
minimum / maximum   number
minLength / maxLength  integer
pattern         "^regex$"
format          "uuid" | "date-time" | "email"  (advisory)
```

### nlohmann/json one-liners

```cpp
json j = json::parse(str);          // throws parse_error
j.at("key").get<std::string>();     // throws out_of_range / type_error
j["key"] = 42;                      // inserts if missing
j.contains("key");                  // safe check
j.dump();                           // serialize
j.dump(2);                          // pretty-print, indent=2
json arr = json::array();
arr.push_back({{"k","v"}});
```

### Parser selection

| Scenario | Pick |
|---|---|
| Dev tooling, config parsing | nlohmann/json |
| High-throughput pipeline | simdjson |
| Tight memory, no heap | RapidJSON (in-place) |
| Drop JSON entirely | FlatBuffers (zero-copy) / Protobuf (schema evolution) |

### Alternatives summary

| Format | Binary | When |
|---|---|---|
| Protobuf | Yes | Cross-language, schema evolution, gRPC |
| FlatBuffers | Yes | Zero-copy embedded reads |
| MessagePack | Yes | JSON structure, binary encoding |
| CBOR | Yes | IoT, IETF standards (CoAP) |