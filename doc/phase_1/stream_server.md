# stream_server — Architecture Reference

> Machine-readable reference for LLM context.
> Covers `app/stream_server.cpp`: settings, logging, data ports, threading model, GL capture, and invariants.

---

## Build

| Property | Value |
|---|---|
| Source | `app/stream_server.cpp` |
| CMake target | `stream_server` |
| Link libs | `imgui glfw OpenGL::GL Threads::Threads nlohmann_json::nlohmann_json spdlog::spdlog_header_only` |
| Settings file | `stream_server_settings.json` (working directory) |
| Log file | `stream_server.log` (plain text, no ANSI) |
| Listen port | `g_settings->port` (default 9999) |
| macOS define | `GL_SILENCE_DEPRECATION` |

---

## Infrastructure

### Settings — `ServerSettings` / `g_settings`

```cpp
struct ServerSettings {
    int         port     = 9999;
    std::string logLevel = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ServerSettings, port, logLevel)
};
static SettingsItem<ServerSettings> g_settings("StreamServerSettings");
```

`SettingsItem<ServerSettings>` self-registers with `SettingsRegistry` at static-init time.
`SettingsRegistry::instance().loadJson("stream_server_settings.json")` is called first in `main()`
before any other initialization; missing file is silently skipped and defaults apply.
`saveJson(...)` is called after GLFW/ImGui teardown on clean exit to persist last-used values.

| Parameter | Default | Effect |
|---|---|---|
| `port` | `9999` | TCP port the server listens on; read once by `netThread()` at start |
| `logLevel` | `"info"` | Passed to `Log::init()` — controls spdlog minimum level |

---

### Logging

```cpp
Log::init(g_settings->logLevel, "stream_server.log");
auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
Log::addSink(imguiSink);
auto log = Log::get("server");   // main / render thread logger
// net thread uses: auto log = Log::get("net");
```

`Log::init` creates two sinks shared by all named loggers:
- `ansicolor_stdout_sink_mt` — always ANSI colors in the terminal
- `basic_file_sink_mt("stream_server.log")` — plain text

`ImGuiLogSink_mt` is a third sink added via `Log::addSink`. It stores log entries in a ring buffer
and renders them in the dockable "Log" ImGui panel (`imguiSink->draw("Log")`).

Level color coding in the ImGui panel: trace=gray, debug=light-blue, info=green, warn=yellow,
error=red, critical=magenta.

Named loggers used:
- `"server"` — render/main thread
- `"net"` — network thread

---

## Threads

| Thread | Created by | Entry point |
|---|---|---|
| Render Thread | OS (main) | `main()` |
| Net Thread | `std::thread netThr(netThread)` | `netThread()` |

### GL Context Ownership

`main()` calls `glfwMakeContextCurrent(window)` — the **Render Thread** owns the OpenGL context.
All GL calls (`glGenTextures`, `glBindTexture`, `glTexImage2D`, `glReadPixels`,
`glViewport`, `glClear`, `ImGui_ImplOpenGL3_*`) are in `main()`.
`netThread()` contains **zero GL calls**.

---

## Data Structures

### `FrameData` — frame payload type

Carried through the render→net frame data port.

```cpp
struct FrameData {
    std::vector<uint8_t> pixels;   // RGB, top-left origin (flipped from GL)
    int                  width  = 0;
    int                  height = 0;
};
```

### `NetStatsData` — stats payload type

Carried through the net→render stats data port.

```cpp
struct NetStatsData {
    uint64_t    bytesSent  = 0;
    uint64_t    framesSent = 0;
    bool        connected  = false;
    std::string clientAddr;
};
```

---

## Data Port Wiring

```
Render Thread (producer)               Net Thread (consumer)
─────────────────────────────────────────────────────────────
g_frameSender ──[g_framePool 3 slots]──▶ g_frameReceiver
                                             (global, used in netThread)

Net Thread (producer)                  Render Thread (consumer)
─────────────────────────────────────────────────────────────
g_statsSender ──[g_statsPool 2 slots]──▶ statsReceiver
                                             (local in main)
```

| Port | Owner | Direction | Pool size |
|---|---|---|---|
| `g_frameSender` | Render Thread | render → net | 3 slots |
| `g_frameReceiver` | Net Thread | render → net (consume side) | — |
| `g_statsSender` | Net Thread | net → render | 2 slots |
| `statsReceiver` (local) | Render Thread | net → render (consume side) | — |

**Wiring in `main()`:**
```cpp
g_frameSender.connectMempool(g_framePool);
g_frameReceiver.connect(g_frameSender);

dc::ReceiverPort<NetStatsData> statsReceiver;
statsReceiver.connect(g_statsSender);
```

**Wiring in `netThread()`:**
```cpp
g_statsSender.connectMempool(g_statsPool);
```

**Latest-wins semantics:** If the render thread produces faster than the net thread consumes,
the newest delivered slot wins. If `reserve()` returns nullptr (all 3 slots held), the render
thread logs a warning and skips capture that frame entirely.

---

## Condvar (frame availability notification)

```cpp
static std::mutex              g_frameCvMtx;
static std::condition_variable g_frameCv;
```

`ReceiverPort` has no built-in blocking mechanism — it is designed for polling loops.
The condvar bridges the gap: the Render Thread calls `g_frameCv.notify_one()` immediately
after `g_frameSender.deliver()`. The Net Thread calls `g_frameCv.wait_for(lk, 200ms)`
**without a predicate** at the top of its inner receive loop. Any notify wakes it immediately;
the 200 ms timeout is a safety net for checking `g_running` if no frame ever arrives.

The data port carries the actual frame data. The condvar is notification only.

On shutdown: `g_frameCv.notify_all()` is called after `g_running.store(false)` to guarantee
the net thread exits its `wait_for` immediately.

---

## Atomics and Globals

| Variable | Type | Initial | Writer | Reader | Notes |
|---|---|---|---|---|---|
| `g_running` | `atomic<bool>` | `false` | Render: `true` before launch, `false` on close | Net: loop condition | Global stop signal |
| `g_frameCvMtx` | `std::mutex` | — | — | Net: `unique_lock` for condvar | Held only during `wait_for` |
| `g_frameCv` | `condition_variable` | — | Render: `notify_one` / `notify_all` | Net: `wait_for` | Notification only; no predicate |

---

## Free Functions

### `send_all(sock_t fd, const void* buf, size_t n) → bool`

**Caller:** Net Thread only.

Loop-calls `::send()` until all `n` bytes are sent or an error occurs.
Returns `false` when `send() <= 0`.
`SIGPIPE` is suppressed on POSIX via `signal(SIGPIPE, SIG_IGN)`.

---

### `send_frame(sock_t fd, const uint8_t* pixels, int w, int h) → bool`

**Caller:** Net Thread only.

Builds and sends the wire header, then the pixel data.

Wire format: `[4B width BE][4B height BE][4B payload_size BE][pixels RGB]`

---

### `deliverStats(bytesSent, framesSent, connected, clientAddr) → void`

**Caller:** Net Thread only.

Pushes a `NetStatsData` snapshot to the render thread via the stats data port.
If `reserve()` returns nullptr (pool exhausted) the update is silently dropped.

```
slot = g_statsSender.reserve()
if !slot: return
slot->bytesSent  = bytesSent
slot->framesSent = framesSent
slot->connected  = connected
slot->clientAddr = clientAddr
g_statsSender.deliver()
```

---

### `netThread()` — Net Thread entry

```
log = Log::get("net")
g_statsSender.connectMempool(g_statsPool)
port = g_settings->port

srv = socket(AF_INET, SOCK_STREAM, 0)
setsockopt(srv, SO_REUSEADDR, 1)
bind(srv, INADDR_ANY:port)
listen(srv, 1)
log.info "Listening on TCP :{port}"

while g_running:
    select(srv, timeout=1s)          // non-blocking accept poll; recheck g_running
    if timeout: continue

    client = accept(srv)
    addrStr = clientIp + ":" + clientPort
    log.info "Client connected: {addrStr}"
    deliverStats(0, 0, connected=true, addrStr)

    bytesSent = 0; framesSent = 0

    while g_running:
        unique_lock lk(g_frameCvMtx)
        g_frameCv.wait_for(lk, 200ms)    // no predicate; woken by notify_one or timeout
        if !g_running: break

        g_frameReceiver.update()
        if !g_frameReceiver.hasNewData():
            g_frameReceiver.cleanup()
            continue

        f = g_frameReceiver.getData()
        if f is empty/invalid:
            g_frameReceiver.cleanup()
            continue

        if !send_frame(client, f->pixels.data(), f->width, f->height):
            log.warn "send_frame failed — client disconnected"
            g_frameReceiver.cleanup()
            break

        bytesSent  += 12 + f->width * f->height * 3
        framesSent++
        g_frameReceiver.cleanup()
        deliverStats(bytesSent, framesSent, true, addrStr)

        if framesSent % 300 == 0:
            log.debug "Sent {} frames ({:.2f} MB)"

    close_sock(client)
    log.info "Client disconnected: {addrStr}"
    deliverStats(0, 0, connected=false)

close_sock(srv)
log.info "Net thread exited"
```

---

### `main()` — Render Thread

**Startup:**

1. POSIX: `signal(SIGPIPE, SIG_IGN)`. Win32: `WSAStartup`.
2. `SettingsRegistry::instance().loadJson("stream_server_settings.json")`.
3. `Log::init(g_settings->logLevel, "stream_server.log")` + `ImGuiLogSink` + `Log::addSink`.
4. `glfwInit()`, create 1280×720 window titled "Stream Server".
5. `glfwMakeContextCurrent(window)`, `glfwSwapInterval(1)` (vsync on).
6. `IMGUI_CHECKVERSION()`, `ImGui::CreateContext()`, `ImGuiConfigFlags_DockingEnable`.
7. `ImGui_ImplGlfw_InitForOpenGL(window, true)`, `ImGui_ImplOpenGL3_Init(glslVersion)`.
8. `glGenTextures(1, &previewTex)` — GL_LINEAR, GL_CLAMP_TO_EDGE. `previewReady=false`, `previewW=0`, `previewH=0`.
9. Wire frame port: `g_frameSender.connectMempool(g_framePool)` → `g_frameReceiver.connect(g_frameSender)`.
10. Wire stats port: local `dc::ReceiverPort<NetStatsData> statsReceiver`; `statsReceiver.connect(g_statsSender)`.
11. `g_running.store(true)`, `std::thread netThr(netThread)`.

**Per-frame render loop (order is critical for glReadPixels):**

```
1.  glfwPollEvents

2.  STATS PULL (data port — no lock):
    statsReceiver.update()
    if statsReceiver.hasNewData():
        s = statsReceiver.getData()
        bytesSent  = s->bytesSent
        framesSent = s->framesSent
        connected  = s->connected
        clientAddr = s->clientAddr
    statsReceiver.cleanup()

3.  Render FPS EMA (α = 0.05):
    dt = clock::now() - lastFrameTime; lastFrameTime = now
    renderFps = renderFps * 0.95 + (1/dt) * 0.05

4.  ImGui_ImplOpenGL3_NewFrame()
    ImGui_ImplGlfw_NewFrame()
    ImGui::NewFrame()

5.  Full-screen DockSpace

6.  ImGui::Begin("Server Control")
    — Render FPS
    — Port and log level from g_settings (read-only display)
    — If connected: green "Connected: {clientAddr}", frames sent, MB sent
    — Else: orange "Waiting for client on :{port} …"
    — Protocol format hint
    ImGui::End()

7.  ImGui::Begin("Captured Preview")
    — If previewReady: aspect-ratio-preserved ImGui::Image(previewTex, ...)
    — Else: "Waiting for first frame…"
    ImGui::End()

8.  imguiSink->draw("Log")

9.  ImGui::Render()
    glfwGetFramebufferSize(window, &fbW, &fbH)
    glViewport / glClearColor(0.10, 0.10, 0.14) / glClear
    ImGui_ImplOpenGL3_RenderDrawData   ← back buffer now contains the current frame

10. CAPTURE + DELIVER (after render, before swap):
    slot = g_frameSender.reserve()
    if slot:
        slot->pixels.resize(fbW * fbH * 3)
        glReadPixels(0, 0, fbW, fbH, GL_RGB, GL_UNSIGNED_BYTE, slot->pixels.data())
        vertical flip in-place (swap_ranges row 0↔fbH-1, …)
        slot->width = fbW; slot->height = fbH
        // Upload previewTex BEFORE deliver() — sender still owns slot
        glBindTexture(GL_TEXTURE_2D, previewTex)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbW, fbH, 0, GL_RGB, GL_UNSIGNED_BYTE, slot->pixels.data())
        previewReady = true; previewW = fbW; previewH = fbH
        g_frameSender.deliver()
        g_frameCv.notify_one()
    else:
        log.warn "Frame pool exhausted — skipping capture this frame"

11. glfwSwapBuffers
```

**Shutdown:**

```
log.info "Shutting down"
g_running.store(false)
g_frameCv.notify_all()   // unblock net thread if blocked in wait_for
netThr.join()
glDeleteTextures(1, &previewTex)
ImGui_ImplOpenGL3_Shutdown() / ImGui_ImplGlfw_Shutdown() / ImGui::DestroyContext()
glfwDestroyWindow / glfwTerminate
SettingsRegistry::instance().saveJson("stream_server_settings.json")
log.info "Settings saved. Exiting."
```

---

## GL Capture and Preview Texture

| Step | What | Why |
|---|---|---|
| `glReadPixels` | Read back buffer into `slot->pixels` | Synchronous; called after `RenderDrawData`, before `SwapBuffers` |
| Vertical flip | `swap_ranges` row 0↔(fbH-1) | GL origin is bottom-left; wire/encoder expects top-left |
| `glTexImage2D(previewTex)` | Upload flipped pixels to preview texture | Done **before** `deliver()` — sender still owns the slot |
| `g_frameSender.deliver()` | Fan-out slot to `g_frameReceiver` | After this the slot is owned by the receiver |
| `g_frameCv.notify_one()` | Wake net thread | Net thread was sleeping in `wait_for(200ms)` |

Preview texture format: `GL_RGB`, internal `GL_RGB`, type `GL_UNSIGNED_BYTE`.
Filter: `GL_LINEAR`. Wrap: `GL_CLAMP_TO_EDGE`.

---

## Key Invariants

1. **Capture order:** `glReadPixels` must execute after `ImGui_RenderDrawData` and before
   `glfwSwapBuffers`. Violating this produces black or stale frames.

2. **No GL in Net Thread:** All `glTex*`, `glReadPixels`, `glViewport`, `glClear` are in `main()`.

3. **Preview before deliver:** `glTexImage2D(previewTex, ..., slot->pixels.data())` is called
   between `reserve()` and `deliver()`. The sender owns the slot in that window — safe to read.

4. **Stats port is drop-tolerant:** `deliverStats` silently returns if `reserve()` returns nullptr.
   Stats are best-effort; losing a frame-stat update is harmless.

5. **Condvar is notification only:** `wait_for` has no predicate. The data port carries actual
   frame data; the condvar is only an efficiency mechanism to avoid busy-polling.

6. **Single active client:** `listen(srv, 1)`. The net thread serves one client at a time.

7. **Settings read-once:** `g_settings->port` is read at `netThread()` startup, not per-accept.
   Changing the JSON while the server is running has no effect.

8. **Frame pool exhaustion:** If `reserve()` returns nullptr (all 3 slots held), `glReadPixels`
   is skipped entirely that frame. No partial delivery.

9. **Vertical flip:** OpenGL origin is bottom-left. The flip happens CPU-side after capture.
   All downstream code receives top-left data.

---

## Timing Parameters

| Parameter | Value | Purpose |
|---|---|---|
| `glfwSwapInterval` | 1 (vsync) | Caps render rate to monitor refresh |
| `select` timeout | 1 s | Max wait between `accept()` calls; allows `g_running` check |
| Condvar `wait_for` timeout | 200 ms | Net thread safety net for `g_running` check |
| Stats log interval | every 300 frames | `log->debug` bandwidth report |
| Render FPS EMA α | 0.05 | `renderFps = renderFps*0.95 + (1/dt)*0.05` |
| Frame pool slots | 3 | 1 being filled by render, 1 pending in receiver, 1 active in receiver |
| Stats pool slots | 2 | 1 being filled by net thread, 1 active in render thread |
