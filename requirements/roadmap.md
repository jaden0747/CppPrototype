# Sunshine & Moonlight: Hands-On Streaming Pipeline Learning Plan

## The Full Pipeline at a Glance

```
[Sunshine Server]                    [Moonlight Client]
Capture → Encode → Packetize → TX ══ RX → Reassemble → Decode → Render
                                 ↕
                          Input (reverse)
```

---

## Phase 1: Raw Frame Streaming (No Encoding)

**Goal:** Send raw video frames over the network and display them. Understand the producer-consumer loop.

**How Sunshine does it:** Sunshine's capture thread writes frames into a ring buffer; a separate encode thread consumes them. The boundary between capture and encode is a lock-free queue.
Relevant files: `src/platform/linux/kmsgrab.cpp`, `src/video.cpp` (the `capture_thread`).

**Your simplified version:** Python, TCP, raw BGR frames from a webcam or screen.

### Step 1A — Server (capture + transmit)

```python
# server.py  — requires: pip install opencv-python
import cv2, socket, struct, time

HOST, PORT = "0.0.0.0", 9999
WIDTH, HEIGHT = 640, 480
FPS_TARGET = 30

cap = cv2.VideoCapture(0)          # webcam; swap for mss for screen capture
cap.set(cv2.CAP_PROP_FRAME_WIDTH,  WIDTH)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, HEIGHT)

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((HOST, PORT))
server.listen(1)
print(f"Waiting for client on {PORT}…")
conn, addr = server.accept()
print(f"Connected: {addr}")

frame_interval = 1.0 / FPS_TARGET
try:
    while True:
        t0 = time.monotonic()
        ret, frame = cap.read()
        if not ret:
            break

        # Simple framing protocol: [4-byte length][raw BGR bytes]
        data = frame.tobytes()
        header = struct.pack(">I", len(data))   # big-endian uint32
        conn.sendall(header + data)

        elapsed = time.monotonic() - t0
        time.sleep(max(0, frame_interval - elapsed))
finally:
    cap.release()
    conn.close()
```

### Step 1B — Client (receive + render)

```python
# client.py  — requires: pip install opencv-python numpy
import cv2, socket, struct, numpy as np

HOST, PORT = "127.0.0.1", 9999
WIDTH, HEIGHT = 640, 480

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))
print("Connected to server")

def recv_exact(sock, n):
    """Block until exactly n bytes are read."""
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("Server closed connection")
        buf.extend(chunk)
    return bytes(buf)

try:
    while True:
        header = recv_exact(sock, 4)
        length = struct.unpack(">I", header)[0]
        raw    = recv_exact(sock, length)
        frame  = np.frombuffer(raw, dtype=np.uint8).reshape((HEIGHT, WIDTH, 3))
        cv2.imshow("Stream", frame)
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
finally:
    sock.close()
    cv2.destroyAllWindows()
```

**Verify it works:** Run server then client. You should see a live webcam window. Measure RTT with `ping` — your frame latency is at minimum 2× that plus render time.

**Key insight:** You've just built the producer-consumer loop. The raw frame size for 640×480 BGR is ~900 KB. At 30 FPS that's **~27 MB/s** — unworkable on real networks. This is *why encoding exists*.

---

## Phase 2: H.264 Encoding & Decoding

**Goal:** Compress frames with H.264 before sending. Understand keyframes, PTS, and codec parameters.

**How Sunshine does it:** Sunshine uses FFmpeg's `libavcodec` with NVENC/VAAPI/x264 backends. It configures the encoder with: zero-latency tune, `intra-refresh` instead of keyframes (to avoid large I-frame spikes), and `zerolatency` preset.
Key files: `src/video.cpp` → `encode_thread`, `src/platform/linux/vaapi.cpp`.

**The crucial settings Sunshine uses (and why):**
- `preset=ultrafast` or `p1` (NVENC) — minimize encode latency
- `tune=zerolatency` — disable lookahead buffering
- `intra-refresh` — spread I-frame cost across multiple frames
- `sliced-threads` — reduce pipeline delay

### Step 2A — Encoding server (C++)

```cpp
// encode_server.cpp
// Build: g++ encode_server.cpp -o encode_server \
//        $(pkg-config --cflags --libs libavcodec libavutil libavformat) \
//        $(pkg-config --cflags --libs opencv4) -lz

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <opencv2/opencv.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

static constexpr int WIDTH   = 1280;
static constexpr int HEIGHT  = 720;
static constexpr int FPS     = 30;
static constexpr int BITRATE = 4'000'000;   // 4 Mbps

// ── Framing: [4-byte big-endian packet length][NAL bytes] ──────────────────

void send_packet(int fd, AVPacket* pkt) {
    uint32_t len = htonl(static_cast<uint32_t>(pkt->size));
    send(fd, &len, 4, MSG_NOSIGNAL);
    send(fd, pkt->data, pkt->size, MSG_NOSIGNAL);
}

int main() {
    // ── 1. Set up encoder ─────────────────────────────────────────────────
    const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) { std::cerr << "libx264 not found\n"; return 1; }

    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    ctx->width        = WIDTH;
    ctx->height       = HEIGHT;
    ctx->time_base    = {1, FPS};
    ctx->framerate    = {FPS, 1};
    ctx->bit_rate     = BITRATE;
    ctx->pix_fmt      = AV_PIX_FMT_YUV420P;
    ctx->gop_size     = FPS * 2;       // keyframe every 2 s
    ctx->max_b_frames = 0;             // no B-frames → lower latency

    // Zero-latency options — mirrors Sunshine's video.cpp
    av_opt_set(ctx->priv_data, "preset",      "ultrafast", 0);
    av_opt_set(ctx->priv_data, "tune",        "zerolatency", 0);
    av_opt_set(ctx->priv_data, "x264-params",
        "intra-refresh=1:slice-max-size=1500:nal-hrd=cbr", 0);

    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        std::cerr << "Failed to open codec\n"; return 1;
    }

    // ── 2. BGR → YUV420P converter (OpenCV gives us BGR) ──────────────────
    SwsContext* sws = sws_getContext(
        WIDTH, HEIGHT, AV_PIX_FMT_BGR24,
        WIDTH, HEIGHT, AV_PIX_FMT_YUV420P,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    AVFrame* frame = av_frame_alloc();
    frame->format  = AV_PIX_FMT_YUV420P;
    frame->width   = WIDTH;
    frame->height  = HEIGHT;
    av_image_alloc(frame->data, frame->linesize, WIDTH, HEIGHT,
                   AV_PIX_FMT_YUV420P, 32);

    AVPacket* pkt = av_packet_alloc();

    // ── 3. TCP listener ───────────────────────────────────────────────────
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(9999);
    bind(srv, (sockaddr*)&addr, sizeof(addr));
    listen(srv, 1);
    std::cout << "Waiting for client…\n";
    int client = accept(srv, nullptr, nullptr);
    std::cout << "Client connected\n";

    // ── 4. Capture → encode → send loop ──────────────────────────────────
    cv::VideoCapture cap(0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);

    cv::Mat bgr;
    int64_t pts = 0;
    while (cap.read(bgr)) {
        // Convert BGR → YUV420P
        const uint8_t* src_data[1] = { bgr.data };
        int src_stride[1]          = { (int)bgr.step };
        sws_scale(sws, src_data, src_stride, 0, HEIGHT,
                  frame->data, frame->linesize);
        frame->pts = pts++;

        // Encode
        if (avcodec_send_frame(ctx, frame) < 0) break;
        while (avcodec_receive_packet(ctx, pkt) == 0) {
            send_packet(client, pkt);
            av_packet_unref(pkt);
        }
    }

    close(client); close(srv);
    avcodec_free_context(&ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    sws_freeContext(sws);
    return 0;
}
```

### Step 2B — Decoding client (Python)

```python
# decode_client.py  — pip install pyav opencv-python numpy
import av, socket, struct, numpy as np, cv2, io

HOST, PORT = "127.0.0.1", 9999

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

def recv_exact(s, n):
    buf = bytearray()
    while len(buf) < n:
        d = s.recv(n - len(buf))
        if not d: raise ConnectionError
        buf.extend(d)
    return bytes(buf)

codec = av.CodecContext.create("h264", "r")
codec.open()

try:
    while True:
        length = struct.unpack(">I", recv_exact(sock, 4))[0]
        nal    = recv_exact(sock, length)

        # Feed NAL unit into decoder
        packets = codec.parse(nal)
        for pkt in packets:
            frames = codec.decode(pkt)
            for frame in frames:
                img = frame.to_ndarray(format="bgr24")
                cv2.imshow("Decoded Stream", img)
                if cv2.waitKey(1) & 0xFF == ord("q"):
                    raise KeyboardInterrupt
except KeyboardInterrupt:
    pass
finally:
    sock.close()
    cv2.destroyAllWindows()
```

**Verify it works:** Bandwidth should drop from ~27 MB/s to ~0.5 MB/s at 4 Mbps. Check with `iftop` or `nethogs`.

**Key insight:** H.264 NAL units are the unit of transmission. Not frames — *NAL units*. This distinction matters enormously in Phase 3.

---

## Phase 3: UDP Transport & Packetization

**Goal:** Move from TCP to UDP. Handle packet loss, reordering, and MTU limits. This is the heart of low-latency streaming.

**How Sunshine/Moonlight do it:** Moonlight uses the **NVIDIA GameStream protocol** which runs over UDP with a custom reliability layer. Each video frame is split into **shards** (fixed-size UDP datagrams). A subset of shards are **FEC (Forward Error Correction) parity shards** — if you lose up to *k* data shards but receive the parity shards, you can reconstruct the frame. This is implemented using Reed-Solomon erasure coding.

Relevant Moonlight source: `app/src/main/java/com/limelight/nvstream/av/video/VideoDepacketizer.java`, `app/src/main/java/com/limelight/nvstream/av/RtpPacket.java`

Sunshine source: `src/rtp.cpp`, `src/rtsp.cpp`

### The Packet Format (simplified from GameStream)

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          sequence_num         |           frame_index         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  shard_index  | total_shards  |  fec_shards   |     flags     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         payload...                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### Step 3A — UDP packetizer server

```python
# udp_server.py  — pip install opencv-python zfec
# zfec provides Reed-Solomon FEC (same algorithm Moonlight uses)
import cv2, socket, struct, time, subprocess, io
import zfec        # pip install zfec

HOST      = "127.0.0.1"
PORT      = 9998
MTU       = 1400   # safe UDP payload (below ethernet MTU of 1500)
FEC_RATIO = 0.25   # 25% redundancy parity shards

def encode_frame_h264(frame_bgr, encoder_state):
    """Thin wrapper around ffmpeg subprocess for simplicity."""
    proc = encoder_state.get("proc")
    if proc is None:
        h, w = frame_bgr.shape[:2]
        cmd = [
            "ffmpeg", "-f", "rawvideo", "-pix_fmt", "bgr24",
            "-s", f"{w}x{h}", "-r", "30", "-i", "pipe:0",
            "-c:v", "libx264", "-preset", "ultrafast",
            "-tune", "zerolatency", "-f", "h264", "pipe:1"
        ]
        proc = subprocess.Popen(cmd, stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        encoder_state["proc"] = proc

    proc.stdin.write(frame_bgr.tobytes())
    proc.stdin.flush()
    import select
    nal_data = b""
    while select.select([proc.stdout], [], [], 0.001)[0]:
        nal_data += proc.stdout.read(65536)
    return nal_data

def packetize(frame_data: bytes, frame_index: int, seq_start: int):
    """Split frame into MTU-sized shards with FEC."""
    if not frame_data:
        return [], seq_start

    chunk_size = MTU - 8   # 8 bytes header
    chunks = [frame_data[i:i+chunk_size]
              for i in range(0, len(frame_data), chunk_size)]

    k = len(chunks)                           # data shards
    m = max(1, int(k * FEC_RATIO))            # parity shards
    total = k + m

    # Pad all chunks to same size for FEC
    max_len = max(len(c) for c in chunks)
    padded  = [c.ljust(max_len, b"\x00") for c in chunks]

    # Generate FEC parity shards
    encoder = zfec.Encoder(k, total)
    all_shards = encoder.encode(padded)

    packets = []
    for i, shard in enumerate(all_shards):
        flags = 0x01 if i == 0 else 0x00
        if i == len(all_shards) - 1:
            flags |= 0x02
        header = struct.pack(">HHBBBB",
            seq_start + i,
            frame_index,
            i,
            total,
            m,
            flags)
        packets.append(header + shard)

    return packets, seq_start + total

def main():
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = (HOST, PORT)

    encoder_state = {}
    frame_index   = 0
    seq_num       = 0
    frame_interval = 1.0 / 30

    print(f"Streaming UDP to {dest} …")
    while True:
        t0 = time.monotonic()
        ret, frame = cap.read()
        if not ret: break

        nal_data = encode_frame_h264(frame, encoder_state)
        if nal_data:
            packets, seq_num = packetize(nal_data, frame_index, seq_num)
            for pkt in packets:
                sock.sendto(pkt, dest)
            frame_index += 1

        elapsed = time.monotonic() - t0
        time.sleep(max(0, frame_interval - elapsed))

if __name__ == "__main__":
    main()
```

### Step 3B — UDP reassembler client with FEC

```python
# udp_client.py  — pip install zfec opencv-python pyav numpy
import socket, struct, collections, time
import zfec, av, cv2, numpy as np

PORT = 9998

class FrameAssembler:
    """Collect shards for one frame; reconstruct with FEC if needed."""
    def __init__(self):
        self.frames     = collections.defaultdict(dict)
        self.frame_meta = {}

    def add_shard(self, frame_idx, shard_idx, total, parity, data):
        self.frames[frame_idx][shard_idx] = data
        self.frame_meta[frame_idx] = (total, parity)
        return self.try_reconstruct(frame_idx)

    def try_reconstruct(self, frame_idx):
        shards   = self.frames[frame_idx]
        total, m = self.frame_meta[frame_idx]
        k        = total - m

        if len(shards) < k:
            return None

        available_indices = sorted(shards.keys())[:k]
        available_data    = [shards[i] for i in available_indices]

        if set(range(k)).issubset(shards):
            frame_data = b"".join(shards[i] for i in range(k))
        else:
            decoder = zfec.Decoder(k, total)
            reconstructed = decoder.decode(available_data, available_indices)
            frame_data = b"".join(reconstructed)

        del self.frames[frame_idx]
        del self.frame_meta[frame_idx]
        return frame_data.rstrip(b"\x00")

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", PORT))
    sock.settimeout(5.0)

    assembler = FrameAssembler()
    codec     = av.CodecContext.create("h264", "r")
    codec.open()

    print(f"Listening on UDP :{PORT}")
    while True:
        try:
            data, _ = sock.recvfrom(65536)
        except socket.timeout:
            continue

        seq, frame_idx, shard_idx, total, parity, flags = \
            struct.unpack(">HHBBBB", data[:8])
        payload = data[8:]

        frame_data = assembler.add_shard(
            frame_idx, shard_idx, total, parity, payload)

        if frame_data:
            pkts = codec.parse(frame_data)
            for pkt in pkts:
                for vframe in codec.decode(pkt):
                    img = vframe.to_ndarray(format="bgr24")
                    cv2.imshow("UDP Stream (FEC)", img)
                    if cv2.waitKey(1) & 0xFF == ord("q"):
                        return

if __name__ == "__main__":
    main()
```

**Verify it works:** Run both sides. To test FEC recovery, add this to the server loop to simulate 10% packet loss:

```python
import random
if random.random() > 0.10:   # drop 10%
    sock.sendto(pkt, dest)
```

The stream should survive without visible corruption.

---

## Phase 4: RTSP Control Channel & Session Negotiation

**Goal:** Add a control plane separate from the data plane. Understand how Moonlight/Sunshine negotiate codec, resolution, and bitrate before streaming starts.

**How Sunshine does it:** Sunshine implements a subset of RTSP (Real-Time Streaming Protocol) for session setup. Moonlight sends `DESCRIBE → SETUP → PLAY` commands; Sunshine responds with SDP (Session Description Protocol) payloads describing the stream parameters.
Source: `src/rtsp.cpp`, `src/network.cpp`.

```
Client (Moonlight)                 Server (Sunshine)
    |                                     |
    |── DESCRIBE rtsp://host/video ──────►|
    |◄── 200 OK + SDP payload ───────────|
    |                                     |
    |── SETUP (transport params) ────────►|
    |◄── 200 OK + session token ─────────|
    |                                     |
    |── PLAY ────────────────────────────►|
    |◄── 200 OK ──────────────────────────|
    |                                     |
    |◄══ RTP/UDP video stream ════════════|
    |◄══ RTP/UDP audio stream ════════════|
    |══ input events ════════════════════►|
```

### Step 4 — Minimal RTSP-like control server

```python
# control_server.py — pure stdlib, no extra deps
import socket, threading, json, time

CONTROL_PORT = 48010
VIDEO_PORT   = 9998

class RTSPishServer:
    """
    A simplified RTSP-like control channel.
    Real Sunshine uses full RTSP + SDP; this teaches the same concepts.
    """
    def __init__(self):
        self.sessions = {}   # token → config

    def handle_client(self, conn, addr):
        print(f"[CTRL] New client: {addr}")
        session_token = None
        try:
            while True:
                data = conn.recv(4096).decode("utf-8").strip()
                if not data:
                    break

                lines  = data.split("\n", 1)
                method = lines[0].strip()
                body   = json.loads(lines[1]) if len(lines) > 1 else {}
                print(f"[CTRL] {method}: {body}")

                if method == "DESCRIBE":
                    response = {
                        "status":     200,
                        "codecs":     ["h264", "hevc"],
                        "max_width":  1920,
                        "max_height": 1080,
                        "max_fps":    60,
                        "video_port": VIDEO_PORT,
                    }

                elif method == "SETUP":
                    import secrets
                    session_token = secrets.token_hex(8)
                    config = {
                        "width":   body.get("width",   1280),
                        "height":  body.get("height",   720),
                        "fps":     body.get("fps",       30),
                        "bitrate": body.get("bitrate", 4_000_000),
                        "codec":   body.get("codec",  "h264"),
                    }
                    self.sessions[session_token] = config
                    response = {
                        "status":  200,
                        "session": session_token,
                        "config":  config,
                    }

                elif method == "PLAY":
                    token = body.get("session")
                    if token not in self.sessions:
                        response = {"status": 404, "error": "Session not found"}
                    else:
                        cfg = self.sessions[token]
                        print(f"[CTRL] Starting stream: {cfg}")
                        response = {"status": 200, "message": "Streaming started"}

                elif method == "TEARDOWN":
                    token = body.get("session", session_token)
                    self.sessions.pop(token, None)
                    response = {"status": 200}
                    conn.sendall((json.dumps(response) + "\n").encode())
                    break

                else:
                    response = {"status": 400, "error": "Unknown method"}

                conn.sendall((json.dumps(response) + "\n").encode())
        except Exception as e:
            print(f"[CTRL] Error: {e}")
        finally:
            conn.close()
            print(f"[CTRL] Client disconnected: {addr}")

    def run(self):
        srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind(("0.0.0.0", CONTROL_PORT))
        srv.listen(5)
        print(f"[CTRL] Listening on :{CONTROL_PORT}")
        while True:
            conn, addr = srv.accept()
            t = threading.Thread(target=self.handle_client, args=(conn, addr))
            t.daemon = True
            t.start()

if __name__ == "__main__":
    RTSPishServer().run()
```

```python
# control_client.py — test the control channel
import socket, json

def send_cmd(sock, method, body=None):
    msg = method
    if body:
        msg += "\n" + json.dumps(body)
    sock.sendall((msg + "\n").encode())
    return json.loads(sock.recv(4096).decode().strip())

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 48010))

r = send_cmd(sock, "DESCRIBE")
print("Capabilities:", r)

r = send_cmd(sock, "SETUP", {"width": 1280, "height": 720, "fps": 30})
print("Session:", r)
token = r["session"]

r = send_cmd(sock, "PLAY", {"session": token})
print("Play:", r)

input("Press Enter to stop…")
send_cmd(sock, "TEARDOWN", {"session": token})
sock.close()
```

---

## Phase 5: Input Handling (Reverse Channel)

**Goal:** Send keyboard/mouse events from client to server. This is the *other* direction of the stream.

**How Moonlight does it:** Moonlight captures input events and encrypts them (AES-GCM) before sending over a separate TCP connection. The server (Sunshine) injects them via `uinput` (Linux) or `SendInput` (Windows).
Source: `src/input.cpp`, Moonlight's `InputHandler.java`.

```python
# input_client.py — pip install pynput
from pynput import keyboard, mouse
import socket, struct, json, threading

INPUT_PORT = 48020

class InputClient:
    def __init__(self, host):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.dest = (host, INPUT_PORT)

    def send_event(self, event_type, data):
        payload = json.dumps({"type": event_type, **data}).encode()
        self.sock.sendto(payload, self.dest)

    def start(self):
        def on_key_press(key):
            try:   k = key.char
            except: k = str(key)
            self.send_event("key_down", {"key": k})

        def on_key_release(key):
            try:   k = key.char
            except: k = str(key)
            self.send_event("key_up", {"key": k})

        def on_move(x, y):
            self.send_event("mouse_move", {"x": x, "y": y})

        def on_click(x, y, button, pressed):
            self.send_event("mouse_click",
                {"x": x, "y": y, "button": str(button), "pressed": pressed})

        kl = keyboard.Listener(on_press=on_key_press, on_release=on_key_release)
        ml = mouse.Listener(on_move=on_move, on_click=on_click)
        kl.start(); ml.start()
        kl.join()

ic = InputClient("127.0.0.1")
ic.start()
```

```python
# input_server.py — receives and prints input events (extend to uinput injection)
import socket, json

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 48020))
print("Input server listening…")

while True:
    data, addr = sock.recvfrom(1024)
    event = json.loads(data.decode())
    print(f"[INPUT] {event}")
    # TODO: inject via uinput (Linux) or SendInput (Windows)
    # For uinput: pip install python-uinput, then:
    # device.emit(uinput.KEY_A, 1)  # press A
    # device.emit(uinput.KEY_A, 0)  # release A
```

---

## Phase 6: Full Architecture Comparison

Here's the complete architecture of what you've built vs. what Sunshine/Moonlight do:

| Your Pipeline | Sunshine/Moonlight |
|---|---|
| Control (TCP :48010) | RTSP (TCP :48010) + HTTPS pairing |
| Video (UDP :9998) | RTP/UDP with SRTP encryption |
| Input (UDP :48020) | Encrypted TCP input channel |
| H.264 via libx264 | H.264/HEVC via NVENC/VAAPI/x264 |
| Custom FEC (zfec RS) | Reed-Solomon (same algorithm) |
| Raw shard protocol | GameStream packet format |

---

## What to Study Next

| Topic | Sunshine file | Moonlight file |
|---|---|---|
| NVENC hardware encoding | `src/platform/windows/nvenc.cpp` | N/A (server-side) |
| Audio streaming (Opus) | `src/audio.cpp` | `AudioDepacketizer.java` |
| SRTP encryption | `src/crypto.cpp` | `RtpDecryptor.java` |
| Adaptive bitrate | `src/video.cpp` → `adjust_bitrate` | `VideoStats.java` |
| HEVC/AV1 support | `src/video.cpp` | `MediaCodecDecoderRenderer.java` |
| mDNS discovery | `src/nvhttp.cpp` | `MdnsDiscoveryAgent.java` |

---

## Key Resources

- **GameStream protocol reverse-engineering:** [Moonlight docs](https://github.com/moonlight-stream/moonlight-docs)
- **Sunshine source:** `github.com/LizardByte/Sunshine`
- **NVIDIA GameStream packet format:** captured in Moonlight's `NvApp.java` and `RtpPacket.java`
- **FFmpeg encode/decode API:** `ffmpeg.org/doxygen/trunk/encode_video_8c-example.html`
- **zfec (Reed-Solomon):** `github.com/tahoe-lafs/zfec`

---

## Core Takeaway

> **Low-latency streaming is entirely about pipeline depth.** Every buffer, every queue, every encode option either adds or removes latency. Sunshine's design goal is minimizing that depth at every stage — that's the thread to pull as you read their source code.