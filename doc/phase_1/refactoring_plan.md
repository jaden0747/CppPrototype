# Phase 1 Refactoring Plan

## Verdict

The original plan is directionally good: it correctly identifies duplicated frame types, duplicated socket helpers, duplicated GL texture/capture code, and overloaded `main()` / `netThread()` functions.

I would not implement it exactly as written. The proposed abstraction splits server and client too early (`TcpFrameSender` vs `TcpFrameReceiver`) and puts thread ownership, socket state, frame protocol, stats, and data-port policy into two large classes. That removes duplication from the app files, but it moves much of the coupling into new classes.

The better split is:

1. **Media model**: what a frame is.
2. **Protocol codec**: how a frame becomes bytes and bytes become a frame.
3. **Transport/session**: how bytes move over TCP.
4. **Threaded endpoints**: how the app connects data ports to transport.
5. **GL adapters**: how frames enter/leave OpenGL.
6. **App shell/UI**: settings, windows, panels, and wiring.

This mirrors the way mature media projects separate buffers, protocol/session behavior, and application integration.

## References From Established Projects

### GStreamer

GStreamer treats `GstBuffer` as the basic data-transfer unit. A buffer owns or references memory and carries timing/offset/metadata. Elements connect through source/sink pads; data flows from a source pad to a sink pad. `appsrc` and `appsink` are the explicit application boundary for pushing buffers into or pulling buffers out of a pipeline.

Reference:
- <https://gstreamer.freedesktop.org/documentation/gstreamer/gstbuffer.html>
- <https://gstreamer.freedesktop.org/documentation/application-development/basics/pads.html>
- <https://gstreamer.freedesktop.org/documentation/app/appsrc.html>
- <https://gstreamer.freedesktop.org/documentation/app/index.html>

Takeaway for this project:
- `Frame` should be a small media object, not tied to TCP or GL.
- Data-port boundaries should look like GStreamer's app boundary: application code pushes/pulls frames, while pipeline internals stay hidden.
- A frame should carry enough metadata to validate and process it: width, height, pixel format, stride/payload size.

### FFmpeg

FFmpeg separates container/protocol reading from media payload representation. `av_read_frame()` returns packets from a demuxer; decoded frames are represented separately as `AVFrame`.

Reference:
- <https://www.ffmpeg.org/doxygen/trunk/group__lavf__decoding.html>
- <https://www.ffmpeg.org/doxygen/7.1/structAVFrame.html>

Takeaway for this project:
- Do not blur "wire packet" and "renderable frame".
- Use a `FrameHeader` / `FrameCodec` layer for the TCP byte layout.
- Keep `Frame` as the app-facing decoded/raw image data.

### OBS Studio

OBS separates sources, encoders, services, and outputs. Outputs can receive raw or encoded data, while encoders are separate objects used by outputs.

Reference:
- <https://docs.obsproject.com/reference-outputs>
- <https://docs.obsproject.com/reference-encoders>

Takeaway for this project:
- "Capture", "encode/protocol", and "output transport" should not be one class.
- Even if Phase 1 sends raw RGB, the abstraction should leave room for later compression without rewriting the UI.

### Boost.Asio

Boost.Asio models TCP as a stream and provides composed operations such as `async_read`, which repeatedly reads until a requested amount of data is available or an error occurs. It also documents the important invariant that overlapping reads on one stream must be avoided.

Reference:
- <https://www.boost.org/doc/libs/latest/doc/html/boost_asio/reference/async_read.html>
- <https://www.boost.org/doc/libs/latest/doc/html/boost_asio/reference/async_read/overload5.html>

Takeaway for this project:
- `send_all` / `recv_all` are really "read exact" / "write exact" stream operations.
- The protocol layer should depend on a byte stream, not directly on app globals.
- The session object should own the "one reader / one writer per socket" invariant.

## Design Problems In The Original Plan

1. `std::atomic<SenderStats>` / `std::atomic<ReceiverStats>` is the wrong direction. It only works for trivially copyable structs and may still need non-obvious platform support. Stats should use either individual atomics, a tiny mutex-protected snapshot, or the existing data-port mechanism.

2. `TcpFrameSender` and `TcpFrameReceiver` are too specific. They mix socket lifecycle, frame protocol, data-port backpressure, stats, thread control, and logging. That makes them hard to test without running the full app.

3. `FrameData` is under-specified. Raw RGB happens to be the current format, but the type should say that explicitly. At minimum it needs pixel format and payload validation helpers.

4. The condition variable should not be injected into transport. It is a scheduling detail of the server endpoint. The transport should read/write frames; the endpoint decides how to wait for frames from a port.

5. The plan keeps "headers-only where possible", but this code touches sockets, OpenGL, threads, and platform APIs. Prefer normal `.cpp` files for those. Headers-only is useful for pure data and small inline helpers, not for lifecycle-heavy code.

6. The optional `GlApp` is useful, but it should not be done before the protocol and endpoint split. UI shell extraction has the highest chance of introducing lifecycle bugs while giving the least architectural value.

## Proposed Architecture

```
include/stream/
  frame.hpp             // Frame, PixelFormat, validation helpers
  frame_protocol.hpp    // FrameHeader, encode/decode, payload-size rules
  byte_stream.hpp       // minimal read_exact/write_exact interface or free funcs
  tcp_socket.hpp        // RAII TCP socket/listener wrapper; preferably Asio-backed
  stream_stats.hpp      // stats snapshots and EMA/FPS helper
  frame_ports.hpp       // small helpers around dc::SenderPort/ReceiverPort if needed
  gl_frame_capture.hpp  // GL framebuffer -> Frame producer
  gl_frame_texture.hpp  // Frame consumer -> GL texture
  stream_server.hpp     // threaded endpoint: ReceiverPort<Frame> -> TCP clients
  stream_client.hpp     // threaded endpoint: TCP server -> SenderPort<Frame>

src/stream/
  frame_protocol.cpp
  tcp_socket.cpp
  stream_stats.cpp
  gl_frame_capture.cpp
  gl_frame_texture.cpp
  stream_server.cpp
  stream_client.cpp

app/
  stream_server.cpp     // settings, logging, ports, panels, endpoint wiring
  stream_client.cpp     // settings, logging, ports, panels, endpoint wiring
```

The names intentionally avoid `TcpFrameSender` / `TcpFrameReceiver` as the core abstraction. The core transport should not care whether it is used by the server app or client app.

## Core Types

### `Frame`

```cpp
namespace stream {

enum class PixelFormat : uint8_t {
    Rgb8,
};

struct Frame {
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::Rgb8;

    int bytes_per_pixel() const;
    size_t expected_size() const;
    bool valid() const;
};

inline constexpr size_t max_frame_payload_size = 64u * 1024u * 1024u;

} // namespace stream
```

Rationale: this follows the GStreamer/FFmpeg idea that the media object is separate from transport. It also makes the current RGB assumption explicit.

### `FrameHeader` / Protocol Codec

```cpp
namespace stream {

struct FrameHeader {
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::Rgb8;
    uint32_t payload_size = 0;
};

inline constexpr size_t frame_header_wire_size = 16;

std::array<uint8_t, frame_header_wire_size> encode_header(const FrameHeader& h);
std::optional<FrameHeader> decode_header(std::span<const uint8_t, frame_header_wire_size> bytes);

FrameHeader header_from_frame(const Frame& frame);
bool validate_header(const FrameHeader& h);

} // namespace stream
```

For C++17, replace `std::span` with `const uint8_t*` plus size, or a `std::array<uint8_t, N>`.

Important change from the current wire format: use a 16-byte header and include pixel format:

```
[4B width BE][4B height BE][1B format][3B reserved][4B payload_size BE][payload]
```

If strict wire compatibility matters for Phase 1, keep the current 12-byte header and add `FrameProtocolVersion::RawRgbV1`. But for a refactor plan, I would define the versioned 16-byte format now.

### Byte Stream / Socket

Preferred implementation: use standalone Asio, already present in `CMakeLists.txt`.

```cpp
namespace stream {

class TcpConnection {
public:
    bool read_exact(void* dst, size_t size);
    bool write_exact(const void* src, size_t size);
    void close();
};

class TcpListener {
public:
    explicit TcpListener(uint16_t port);
    std::optional<TcpConnection> accept_for(std::chrono::milliseconds timeout);
    void close();
};

} // namespace stream
```

Why Asio:
- removes duplicated `_WIN32` / POSIX socket shims from the app files;
- matches Boost.Asio's stream model;
- gives a cleaner path to async I/O later;
- still allows synchronous Phase 1 implementation in one worker thread.

If you do not want Asio yet, keep raw sockets behind this same API. The important part is that app code never sees `sock_t`, `select`, `send`, `recv`, or `close`.

### Frame I/O

```cpp
namespace stream {

bool write_frame(TcpConnection& conn, const Frame& frame);

// Resizes `out.pixels` after validating the header, then reads payload.
bool read_frame(TcpConnection& conn, Frame& out);

// Reads and discards a payload when no frame slot is available.
bool drain_frame_payload(TcpConnection& conn, const FrameHeader& header);

} // namespace stream
```

This is the key abstraction. It is testable without GL, ImGui, data ports, or app settings.

## Threaded Endpoints

The endpoint classes are application adapters. They own threads and connect data ports to frame I/O.

### Server Endpoint

```cpp
namespace stream {

struct ServerEndpointConfig {
    uint16_t listen_port = 9999;
    std::chrono::milliseconds frame_wait = std::chrono::milliseconds(200);
};

struct ServerStats {
    uint64_t bytes_sent = 0;
    uint64_t frames_sent = 0;
    bool connected = false;
    std::string peer;
};

class StreamServerEndpoint {
public:
    StreamServerEndpoint(ServerEndpointConfig cfg,
                         dc::ReceiverPort<Frame>& frames,
                         dc::SenderPort<ServerStats>& stats);

    void start();
    void stop();
    void notify_frame_available();

private:
    void run();
};

} // namespace stream
```

`notify_frame_available()` belongs here, not in the socket layer. The endpoint knows it is waiting on a `ReceiverPort`; the TCP connection does not.

### Client Endpoint

```cpp
namespace stream {

struct ClientEndpointConfig {
    std::string server_ip = "127.0.0.1";
    uint16_t server_port = 9999;
};

struct ClientStats {
    uint64_t bytes_received = 0;
    uint64_t frames_received = 0;
    float fps = 0.0f;
    bool connected = false;
    std::string status;
};

class StreamClientEndpoint {
public:
    StreamClientEndpoint(dc::SenderPort<Frame>& frames,
                         dc::SenderPort<ClientStats>& stats);

    void start();
    void stop();
    void connect(ClientEndpointConfig cfg);
    void disconnect();

private:
    void run();
};

} // namespace stream
```

The client endpoint owns connection state and reconnection/disconnect requests. It should not expose global atomics.

## Stats Policy

Use the existing data-port mechanism for stats snapshots. It is already part of the app architecture and can carry strings safely.

Avoid:

```cpp
std::atomic<ServerStats> stats_;
```

Prefer:

```cpp
dc::SenderPort<ServerStats>& stats_;
```

or, for non-port users:

```cpp
class StatsSnapshot {
public:
    void store(ServerStats s);
    ServerStats load() const;
private:
    mutable std::mutex mutex_;
    ServerStats value_;
};
```

Use individual atomics only for small scalar counters where the UI does not need an internally consistent snapshot.

## GL Adapters

### `GlFrameCapture`

Responsibilities:
- allocate/reuse GL preview texture;
- reserve a `Frame` slot from a sender port;
- call `glReadPixels`;
- flip rows;
- upload preview texture before `deliver()`;
- deliver frame and notify endpoint.

It should not know about TCP.

### `GlFrameTexture`

Responsibilities:
- consume latest frame from `dc::ReceiverPort<Frame>`;
- create/update GL texture;
- expose texture id, width, height;
- optionally provide a helper to calculate aspect-fit size.

It should not draw full UI panels. The app file should still own ImGui panels.

## Migration Plan

Each phase should leave a working build.

### Phase A: Media and Protocol

1. Add `include/stream/frame.hpp`.
2. Add `include/stream/frame_protocol.hpp` and `src/stream/frame_protocol.cpp`.
3. Add focused tests for:
   - valid header encode/decode;
   - bad dimensions;
   - bad payload size;
   - payload larger than `max_frame_payload_size`;
   - frame expected-size calculation.
4. Replace duplicated `FrameData` with `stream::Frame`.

Risk: low. This is pure data and validation logic.

### Phase B: Socket Boundary

1. Add `TcpConnection` and `TcpListener`.
2. Implement with standalone Asio if possible; otherwise hide raw sockets behind the same API.
3. Move `send_all` / `recv_all` into `read_exact` / `write_exact`.
4. Keep the old app-level `netThread()` functions, but make them call `read_frame()` / `write_frame()`.

Risk: medium. This touches connection behavior but keeps the thread structure intact.

### Phase C: GL Adapters

1. Add `GlFrameCapture`.
2. Add `GlFrameTexture`.
3. Replace the capture/upload blocks in the two app files.
4. Verify capture order remains:
   - render ImGui;
   - read framebuffer;
   - upload preview while slot is still owned;
   - deliver frame;
   - swap buffers.

Risk: medium. OpenGL state and call order are easy to break.

### Phase D: Threaded Endpoints

1. Add `StreamServerEndpoint`.
2. Add `StreamClientEndpoint`.
3. Move the remaining `netThread()` logic into those endpoints.
4. Remove global connection-control atomics from the app files.
5. Keep stats delivery through data ports.

Risk: medium. Thread shutdown, disconnect, and pool-exhausted behavior must be tested manually.

### Phase E: App Shell, Later

Only after Phases A-D are stable, consider extracting common GLFW/ImGui lifecycle into `GlApp`.

Do not do this first. It gives cosmetic line-count reduction but increases the chance of breaking the render/capture lifecycle.

## Testing Checklist

Add unit tests for protocol logic before touching sockets:

- header round-trip;
- invalid width/height;
- invalid payload size;
- expected RGB payload calculation;
- payload mismatch rejection.

Manual verification after endpoint extraction:

- server starts and waits for a client;
- client connects;
- client receives frames;
- disconnect button returns to disconnected state;
- client can reconnect;
- server exits while no client is connected;
- server exits while client is connected;
- frame pool exhaustion logs but stream remains synchronized.

## Final Recommendation

Use this shape:

```
Frame
  |
FrameProtocol / read_frame / write_frame
  |
TcpConnection / TcpListener
  |
StreamServerEndpoint / StreamClientEndpoint
  |
app/stream_server.cpp and app/stream_client.cpp
```

This is a better abstraction than the original plan because it separates stable concepts from volatile ones:

- frame format can evolve without rewriting socket code;
- TCP can move from blocking to async without rewriting GL code;
- raw RGB can later become compressed packets without rewriting UI panels;
- app files stay responsible for settings, logging, and wiring;
- endpoint classes own thread lifecycle but not media representation.

That is much closer to how GStreamer, FFmpeg, OBS, and Asio draw their boundaries.
