# Design Decisions

This document records design decisions made during implementation where multiple
valid approaches existed. Each entry explains the reasoning and trade-offs.

---

## Phase 4: Control Channel Protocol Format

**Decision:** Use a length-prefixed JSON protocol (`[4-byte big-endian length][JSON]`)
instead of text-line-based RTSP or raw binary protocol.

**Alternatives considered:**
1. **Text-line RTSP** (as in the roadmap's Python example) — `METHOD\n{json}\n`
2. **Binary protocol** with fixed-size message headers
3. **Length-prefixed JSON** (chosen)

**Reasoning:**
- The Python example used newline-delimited text, but that breaks if JSON contains
  newlines. Length-prefixing is more robust.
- Binary protocol would be more efficient but harder to debug and extend.
- JSON provides easy extensibility for future fields (e.g. encryption params,
  audio codec negotiation) without breaking backward compatibility.
- The control channel is low-frequency (a few messages per session), so JSON
  serialization overhead is irrelevant.

**Trade-off:** Slightly more bandwidth per control message (~100 bytes vs ~20 for
binary). Acceptable given control messages happen once per session.

---

## Phase 4: Single-Client Control Server

**Decision:** The control server handles one client at a time (blocking accept loop).

**Alternatives considered:**
1. **Multi-threaded** — one thread per control client
2. **Async I/O** — using asio for non-blocking multiplexing
3. **Single-client blocking** (chosen)

**Reasoning:**
- Sunshine itself only handles one streaming session at a time (one display,
  one encoder). Multi-client would require multi-encoder support.
- The existing `StreamServerEndpoint` and `UdpServerEndpoint` are also single-client.
- Keeping it simple matches the learning roadmap's intent.

**Trade-off:** Cannot support multiple simultaneous viewers. This is acceptable
for a game-streaming architecture where one player has exclusive control.

---

## Phase 5: UDP vs TCP for Input Events

**Decision:** Use UDP for the input reverse channel.

**Alternatives considered:**
1. **TCP** — as Moonlight actually does (encrypted TCP)
2. **UDP** (chosen) — as suggested in the roadmap

**Reasoning:**
- The roadmap explicitly specifies UDP for input events.
- Input events are inherently idempotent: a dropped mouse_move is immediately
  superseded by the next one (~125 Hz). TCP's retransmission and head-of-line
  blocking would add unnecessary latency.
- Key events are more important (a dropped key_down could miss input), but at
  the low loss rates of a LAN, this is acceptable. A production system would add
  a small sequence number and reliability layer for key events only.
- Moonlight uses encrypted TCP for input mainly for security (AES-GCM), not for
  reliability. We don't implement encryption in this learning project.

**Trade-off:** Possible dropped key events under packet loss. Acceptable for the
educational scope; a production system would add selective reliability.

---

## Phase 5: No Native Input Injection

**Decision:** The input receiver logs events but does not inject them via OS APIs
(e.g., `SendInput` on Windows, `uinput` on Linux).

**Alternatives considered:**
1. **Full OS injection** — actually move the cursor, type keys
2. **Log-only** (chosen) — display in ImGui, prove the pipeline works

**Reasoning:**
- OS input injection requires platform-specific code (`SendInput` on Windows,
  `uinput` on Linux) and elevated permissions on some platforms.
- The learning goal is understanding the reverse channel architecture, not
  platform input APIs.
- The receiver exposes a callback (`set_on_event`) so injection can be added
  later without changing the architecture.
- The roadmap's Python example also just prints events with a TODO comment
  for `uinput` injection.

**Trade-off:** The input channel is receive-and-display only. Adding real injection
requires platform-specific implementation but the architecture supports it.

---

## Phase 5: No Integration into rtsp_server/rtsp_client Apps

**Decision:** Phase 5 input handling is implemented as a standalone library
(`stream_input`) rather than being integrated into the Phase 4 RTSP apps.

**Alternatives considered:**
1. **Modify rtsp_server/rtsp_client** to add input panels and handlers
2. **Standalone library only** (chosen)

**Reasoning:**
- The roadmap presents Phase 5 as independent `input_client.py` and
  `input_server.py` scripts, not as modifications to the Phase 4 apps.
- Keeping the library separate follows the existing project pattern where each
  phase has its own library (stream_core, stream_h264, stream_udp, stream_control).
- Integration into a unified app would be a Phase 6+ concern.
- The library's `InputSender`/`InputReceiver` classes can be trivially composed
  into any app that needs them.

**Trade-off:** No single "full pipeline" executable yet. Each phase remains
independently testable, which matches the learning plan's structure.
