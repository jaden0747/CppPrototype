# stream_client — Architecture Reference

> Machine-readable reference for LLM context.
> Covers `app/stream_client.cpp`: settings, logging, data ports, threading model, connection state machine, GL texture management, and invariants.

---

## Build

| Property | Value |
|---|---|
| Source | `app/stream_client.cpp` |
| CMake target | `stream_client` |
| Link libs | `imgui glfw OpenGL::GL Threads::Threads nlohmann_json::nlohmann_json spdlog::spdlog_header_only` |
| Settings file | `stream_client_settings.json` (working directory) |
| Log file | `stream_client.log` (plain text, no ANSI) |
| Connect target | `g_settings->serverIp` : `g_settings->serverPort` (defaults: 127.0.0.1:9999) |
| macOS define | `GL_SILENCE_DEPRECATION` |

---

## Infrastructure

### Settings — `ClientSettings` / `g_settings`

```cpp
struct ClientSettings {
    std::string serverIp   = "127.0.0.1";
    int         serverPort = 9999;
    std::string logLevel   = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ClientSettings, serverIp, serverPort, logLevel)
};
static SettingsItem<ClientSettings> g_settings("StreamClientSettings");
```

`SettingsItem<ClientSettings>` self-registers with `SettingsRegistry` at static-init time.
`SettingsRegistry::instance().loadJson("stream_client_settings.json")` is called first in `main()`
before any other initialization; missing file is silently skipped and defaults apply.
On clean exit, the last-used IP/port values are written back to `g_settings` before `saveJson()`.

| Parameter | Default | Effect |
|---|---|---|
| `serverIp` | `"127.0.0.1"` | Pre-fills the UI InputText; copied to `g_serverIp` at startup |
| `serverPort` | `9999` | Pre-fills the UI InputInt; copied to `g_serverPort` at startup |
| `logLevel` | `"info"` | Passed to `Log::init()` — controls spdlog minimum level |

---

### Logging

```cpp
Log::init(g_settings->logLevel, "stream_client.log");
auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
Log::addSink(imguiSink);
auto log = Log::get("client");   // main / render thread logger
// net thread uses: auto log = Log::get("net");
```

`Log::init` creates two sinks shared by all named loggers:
- `ansicolor_stdout_sink_mt` — always ANSI colors in the terminal
- `basic_file_sink_mt("stream_client.log")` — plain text

`ImGuiLogSink_mt` is a third sink added via `Log::addSink`. It stores log entries in a ring buffer
and renders them in the dockable "Log" ImGui panel (`imguiSink->draw("Log")`).

Level color coding in the ImGui panel: trace=gray, debug=light-blue, info=green, warn=yellow,
error=red, critical=magenta.

Named loggers used:
- `"client"` — render/main thread
- `"net"` — network thread

---

## Threads

| Thread | Created by | Entry point |
|---|---|---|
| Render Thread | OS (main) | `main()` |
| Net Thread | `std::thread netThr(netThread)` | `netThread()` |

### GL Context Ownership

`main()` calls `glfwMakeContextCurrent(window)` — the **Render Thread** owns the OpenGL context.
All GL calls (`glGenTextures`, `glBindTexture`, `glTexImage2D`, `glTexSubImage2D`,
`glViewport`, `glClear`, `ImGui_ImplOpenGL3_*`) are in `main()`.
`netThread()` contains **zero GL calls** — it only writes CPU-side `FrameData` memory.

---

## Data Structures

### `FrameData` — frame payload type

Carried through the net→render frame data port.

```cpp
struct FrameData {
    std::vector<uint8_t> pixels;   // RGB, top-left origin
    int                  width  = 0;
    int                  height = 0;
};
```

### `ClientStatsData` — stats payload type

Carried through the net→render stats data port.

```cpp
struct ClientStatsData {
    uint64_t    bytesRecv  = 0;
    uint64_t    framesRecv = 0;
    float       fps        = 0.0f;
    bool        connected  = false;
    std::string status     = "Disconnected";
};
```

---

## Data Port Wiring

```
Net Thread (producer)                  Render Thread (consumer)
─────────────────────────────────────────────────────────────
g_frameSender ──[g_framePool 3 slots]──▶ frameReceiver
                                             (local in main)

Net Thread (producer)                  Render Thread (consumer)
─────────────────────────────────────────────────────────────
g_statsSender ──[g_statsPool 2 slots]──▶ statsReceiver
                                             (local in main)
```

| Port | Owner | Direction | Pool size |
|---|---|---|---|
| `g_frameSender` | Net Thread | net → render | 3 slots |
| `frameReceiver` (local) | Render Thread | net → render (consume side) | — |
| `g_statsSender` | Net Thread | net → render | 2 slots |
| `statsReceiver` (local) | Render Thread | net → render (consume side) | — |

**Wiring in `netThread()`:**
```cpp
g_frameSender.connectMempool(g_framePool);
g_statsSender.connectMempool(g_statsPool);
```

**Wiring in `main()`:**
```cpp
dc::ReceiverPort<FrameData>       frameReceiver;
dc::ReceiverPort<ClientStatsData> statsReceiver;
frameReceiver.connect(g_frameSender);
statsReceiver.connect(g_statsSender);
```

**Zero-copy receive:** The net thread calls `g_frameSender.reserve()` before `recv_all()`,
then passes `slot->pixels.data()` directly as the `recv` buffer. No intermediate copy.

**Pool-exhausted fallback:** If `reserve()` returns nullptr (all 3 slots held), the net thread
drains the socket bytes into a temporary discard buffer to keep the TCP stream in sync,
logs a warning, and skips the GL upload.

---

## Connection Control

```cpp
static std::string       g_serverIp;      // written by render thread before g_doConnect.store(true)
static int               g_serverPort = 9999;
static std::atomic<bool> g_doConnect{false};
static std::atomic<bool> g_running{false};
```

**`g_serverIp` / `g_serverPort` safety:**
The Render Thread writes both variables, then calls `g_doConnect.store(true, seq_cst)`.
The Net Thread reads both only after observing `g_doConnect.load(seq_cst) == true`.
The sequential-consistency semantics create a happens-before edge from the writes to the reads,
making the non-atomic reads safe without additional locking.

---

## Free Functions

### `recv_all(sock_t fd, void* buf, size_t n) → bool`

**Caller:** Net Thread only.

Loop-calls `::recv()` until all `n` bytes are received or an error occurs.
Returns `false` when `recv() <= 0` (disconnect, reset, etc.).

---

### `deliverStats(bytesRecv, framesRecv, fps, connected, status) → void`

**Caller:** Net Thread only.

Pushes a `ClientStatsData` snapshot to the render thread via the stats data port.
If `reserve()` returns nullptr (pool exhausted) the update is silently dropped.

```
slot = g_statsSender.reserve()
if !slot: return
slot->bytesRecv  = bytesRecv
slot->framesRecv = framesRecv
slot->fps        = fps
slot->connected  = connected
slot->status     = status
g_statsSender.deliver()
```

---

### `netThread()` — Net Thread entry

```
log = Log::get("net")
g_frameSender.connectMempool(g_framePool)
g_statsSender.connectMempool(g_statsPool)

while g_running:
    if !g_doConnect.load():
        sleep(50ms)
        continue

    // Snapshot connection target — written before g_doConnect was set (seq_cst fence)
    ip   = g_serverIp
    port = g_serverPort

    log.info "Connecting to {ip}:{port}…"
    deliverStats(0, 0, 0.0, false, "Connecting to {ip}:{port} …")

    fd = socket(AF_INET, SOCK_STREAM, 0)
    if fd < 0:
        log.error "socket() failed"
        deliverStats(0, 0, 0.0, false, "socket() failed")
        g_doConnect.store(false)
        continue

    inet_pton(AF_INET, ip, &addr.sin_addr)
    if inet_pton returns != 1:
        close_sock(fd)
        log.error "Invalid IP: {ip}"
        deliverStats(0, 0, 0.0, false, "Invalid IP: {ip}")
        g_doConnect.store(false)
        continue

    if connect(fd, &addr) < 0:
        close_sock(fd)
        log.warn "Connection refused to {ip}:{port}"
        deliverStats(0, 0, 0.0, false, "Connection refused")
        g_doConnect.store(false)
        continue

    log.info "Connected to {ip}:{port}"
    deliverStats(0, 0, 0.0, true, "Connected")

    bytesRecv = 0; framesRecv = 0; fps = 0.0

    while g_running && g_doConnect.load():
        if !recv_all(fd, hdr, 12):
            log.warn "recv header failed — server disconnected"
            break

        w   = ntohl(hdr[0..3])
        h   = ntohl(hdr[4..7])
        len = ntohl(hdr[8..11])

        if len == 0 || len > 64*1024*1024:
            log.error "Sanity check failed: len={len} — disconnecting"
            break

        // Zero-copy path: receive directly into mempool slot
        slot = g_frameSender.reserve()
        if slot:
            slot->pixels.resize(len)
            if !recv_all(fd, slot->pixels.data(), len):
                log.warn "recv pixels failed — server disconnected"
                g_frameSender.deliver()   // deliver partial; size guard already validated
                break
            slot->width  = w
            slot->height = h
            g_frameSender.deliver()
        else:
            // Pool exhausted: drain socket to stay in sync
            log.warn "Frame pool exhausted — draining {len} bytes"
            drain = vector<uint8_t>(len)
            if !recv_all(fd, drain.data(), len): break

        dt = clock::now() - lastTime; lastTime = now
        bytesRecv  += 12 + len
        framesRecv++
        if dt > 0: fps = fps * 0.9 + (1/dt) * 0.1   // EMA α = 0.1
        deliverStats(bytesRecv, framesRecv, fps, true, "Connected")

        if framesRecv % 300 == 0:
            log.debug "Received {} frames ({:.2f} MB)"

    close_sock(fd)
    log.info "Disconnected from {ip}:{port}"
    deliverStats(0, 0, 0.0, false, "Disconnected")
    g_doConnect.store(false)

log.info "Net thread exited"
```

---

### `main()` — Render Thread

**Startup:**

1. POSIX: `signal(SIGPIPE, SIG_IGN)`. Win32: `WSAStartup`.
2. `SettingsRegistry::instance().loadJson("stream_client_settings.json")`.
3. `Log::init(g_settings->logLevel, "stream_client.log")` + `ImGuiLogSink` + `Log::addSink`.
4. Initialize connection control: `g_serverIp = g_settings->serverIp`; `g_serverPort = g_settings->serverPort`.
5. `glfwInit()`, create 1280×720 window titled "Stream Client".
6. `glfwMakeContextCurrent(window)`, `glfwSwapInterval(1)` (vsync on).
7. `IMGUI_CHECKVERSION()`, `ImGui::CreateContext()`, `ImGuiConfigFlags_DockingEnable`.
8. `ImGui_ImplGlfw_InitForOpenGL(window, true)`, `ImGui_ImplOpenGL3_Init(glslVersion)`.
9. `glGenTextures(1, &streamTex)` — GL_LINEAR, GL_CLAMP_TO_EDGE. `texW=0`, `texH=0`.
10. Wire data ports: `frameReceiver.connect(g_frameSender)`; `statsReceiver.connect(g_statsSender)`.
11. Initialize UI buffers: `ipBuf` from `g_settings->serverIp`, `portBuf` from `g_settings->serverPort`.
12. `g_running.store(true)`, `std::thread netThr(netThread)`.

**Per-frame render loop:**

```
1.  glfwPollEvents

2.  FRAME UPLOAD (data port → GL, before ImGui frame):
    frameReceiver.update()
    if frameReceiver.hasNewData():
        f = frameReceiver.getData()
        if f && !f->pixels.empty():
            glBindTexture(GL_TEXTURE_2D, streamTex)
            if f->width != texW || f->height != texH:
                glTexImage2D(...)    // reallocate: new dimensions or first frame
                texW = f->width; texH = f->height
            else:
                glTexSubImage2D(...) // same dimensions: update pixels in-place
    frameReceiver.cleanup()

3.  STATS PULL (data port — no lock):
    statsReceiver.update()
    if statsReceiver.hasNewData():
        s = statsReceiver.getData()
        bytesRecv  = s->bytesRecv
        framesRecv = s->framesRecv
        recvFps    = s->fps
        connected  = s->connected
        status     = s->status
    statsReceiver.cleanup()

4.  ImGui_ImplOpenGL3_NewFrame()
    ImGui_ImplGlfw_NewFrame()
    ImGui::NewFrame()

5.  Full-screen DockSpace

6.  ImGui::Begin("Connection")
    — InputText "Server IP" (read-only when connecting or connected)
    — InputInt "Port"       (read-only when connecting or connected)
    — Button: "Connect" | "Connecting…" (disabled) | "Disconnect"
    — Separator
    — Status label (green when connected, orange otherwise)
    — FPS recv, Frames, Data MB, Resolution, Log level
    ImGui::End()

7.  PushStyleVar(WindowPadding, 0,0)
    ImGui::Begin("Video Stream")
    PopStyleVar()
    if texW > 0 && texH > 0:
        aspect-ratio-preserved ImGui::Image(streamTex, ImVec2(dw,dh)), centred
    else:
        ImGui::TextDisabled("No stream — connect to a server")   // centred
    ImGui::End()

8.  imguiSink->draw("Log")

9.  ImGui::Render()
    glfwGetFramebufferSize / glViewport / glClearColor(0.08, 0.08, 0.10) / glClear
    ImGui_ImplOpenGL3_RenderDrawData
    glfwSwapBuffers
```

**Shutdown:**

```
log.info "Shutting down"
g_running.store(false)
g_doConnect.store(false)
netThr.join()
glDeleteTextures(1, &streamTex)
ImGui_ImplOpenGL3_Shutdown() / ImGui_ImplGlfw_Shutdown() / ImGui::DestroyContext()
glfwDestroyWindow / glfwTerminate
// Write last-used IP/port back into settings before saving
g_settings->serverIp   = ipBuf
g_settings->serverPort = portBuf
SettingsRegistry::instance().saveJson("stream_client_settings.json")
log.info "Settings saved. Exiting."
```

---

## GL Texture Management

| Scenario | GL call | Reason |
|---|---|---|
| First frame ever | `glTexImage2D` | Texture has no storage yet — must allocate |
| Resolution changed (`w != texW \|\| h != texH`) | `glTexImage2D` | Old storage has wrong dimensions — reallocate |
| Same resolution as previous frame | `glTexSubImage2D` | Reuses existing GPU storage; avoids reallocation overhead |
| No new frame this vsync (`!hasNewData`) | No GL call | Displays previous frame (last-frame hold) |

Texture format: `GL_RGB`, internal `GL_RGB`, type `GL_UNSIGNED_BYTE`.
Filter: `GL_LINEAR` (min and mag). Wrap: `GL_CLAMP_TO_EDGE` (both axes).

`ImTextureID` cast: `static_cast<ImTextureID>(static_cast<uintptr_t>(streamTex))`.
`reinterpret_cast` is not used because `ImTextureID` is `unsigned long long` in this build;
casting from signed `intptr_t` via `reinterpret_cast` is ill-formed in C++.

---

## Connection State Machine

| Current state | Event | Next state | Side effects |
|---|---|---|---|
| Disconnected | `g_doConnect.load() == false` | Disconnected | Sleep 50 ms, loop |
| Disconnected | UI sets `g_doConnect = true` | Connecting | Snapshot IP/port, update status |
| Connecting | `socket()` fails | Disconnected | `g_doConnect = false`, status error |
| Connecting | `inet_pton()` fails | Disconnected | `close_sock`, `g_doConnect = false`, status "Invalid IP" |
| Connecting | `connect()` fails | Disconnected | `close_sock`, `g_doConnect = false`, status "Connection refused" |
| Connecting | `connect()` succeeds | Connected | `connected = true`, status "Connected" |
| Connected | `recv()` returns ≤ 0 | Disconnected | `close_sock`, `g_doConnect = false`, stats reset |
| Connected | `len > 64 MB` sanity fail | Disconnected | Same as recv failure |
| Connected | UI sets `g_doConnect = false` | Disconnected | Inner loop exits on next iteration |
| Any | `g_running = false` (window close) | shutdown | Net thread exits, `netThr.join()` |

---

## Key Invariants

1. **No GL in Net Thread:** All `glTex*` calls are in the render loop of `main()`.
   The Net Thread writes only to CPU-side `FrameData` Mempool slots.

2. **Texture upload before ImGui frame:** `frameReceiver.update()` and the `glTexSubImage2D`/
   `glTexImage2D` call execute before `ImGui_NewFrame`, ensuring the texture is current before
   `ImGui::Image` samples it in the same frame.

3. **Last-frame hold:** When the Net Thread is slower than vsync or when no new frame arrives,
   `hasNewData()` returns false and the render thread displays the last successfully uploaded
   texture. No wait or block in the render loop.

4. **Zero-copy receive:** The net thread calls `reserve()` before `recv_all()` and passes
   `slot->pixels.data()` directly as the recv buffer. No intermediate copy buffer.

5. **Pool exhausted — stream sync:** If `reserve()` returns nullptr, the socket bytes are drained
   into a temporary discard buffer so the TCP stream stays synchronized with the server.
   Without this, the next header read would land mid-payload.

6. **Atomic ordering for `g_serverIp`:** Render Thread writes non-atomic `g_serverIp`/`g_serverPort`,
   then does `g_doConnect.store(true, seq_cst)`. Net Thread does `g_doConnect.load(seq_cst)`,
   then reads the non-atomics. The seq_cst fence provides the happens-before edge.

7. **Settings persistence:** On exit, `ipBuf` and `portBuf` (the last-used UI values) are written
   back to `g_settings` before `saveJson()`. Next launch pre-fills the UI from these saved values.

8. **Sanity limit:** `len > 64 * 1024 * 1024` triggers disconnect to prevent OOM on corrupt headers.

9. **Aspect-ratio display:** The Video Stream panel scales the image to fill available space
   while preserving aspect ratio (pillarboxed/letterboxed as needed) and centres it.

---

## Timing Parameters

| Parameter | Value | Purpose |
|---|---|---|
| `glfwSwapInterval` | 1 (vsync) | Caps display rate to monitor refresh |
| Net Thread idle poll | 50 ms sleep | How often the Net Thread checks `g_doConnect` while disconnected |
| Receive FPS EMA α | 0.1 | `fps = fps*0.9 + (1/dt)*0.1` — tracks rate with moderate smoothing |
| Stats log interval | every 300 frames | `log->debug` bandwidth report |
| Sanity frame size limit | 64 MB | `len > 64*1024*1024` → disconnect; guards against corrupt header values |
| Frame pool slots | 3 | 1 being filled by net, 1 pending in receiver, 1 active in receiver |
| Stats pool slots | 2 | 1 being filled by net thread, 1 active in render thread |
