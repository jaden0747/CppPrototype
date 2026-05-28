#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# start_encode_stream.sh — Launch encode_server and decode_client (Phase 2)
#
# Usage:
#   ./script/start_encode_stream.sh
#
# The server captures its OpenGL framebuffer, encodes each frame to H.264
# (libx264, ultrafast/zerolatency) and sends NAL bytes over TCP.
# The client receives the NAL stream, decodes via libavcodec, and displays.
# ---------------------------------------------------------------------------
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$WORKSPACE_DIR/build/Debug"

SERVER="$BUILD_DIR/encode_server"
CLIENT="$BUILD_DIR/decode_client"

if [[ ! -x "$SERVER" ]]; then
    echo "ERROR: encode_server not found at $SERVER"
    echo "Build with:"
    echo "  cmake --preset conan-debug && cmake --build build/Debug --target encode_server decode_client"
    exit 1
fi
if [[ ! -x "$CLIENT" ]]; then
    echo "ERROR: decode_client not found at $CLIENT"
    echo "Build with the same command above."
    exit 1
fi

echo "[phase2] Starting encode_server …"
cd "$WORKSPACE_DIR"
"$SERVER" &
SERVER_PID=$!

sleep 1

echo "[phase2] Starting decode_client …"
"$CLIENT" &
CLIENT_PID=$!

echo ""
echo "[phase2] Both running (H.264 pipeline)."
echo "  Server PID : $SERVER_PID"
echo "  Client PID : $CLIENT_PID"
echo ""
echo "  In the client window click Connect to start the stream."
echo "  Close either window or press Ctrl+C here to stop both."
echo ""

cleanup() {
    echo ""
    echo "[phase2] Stopping …"
    kill "$SERVER_PID" "$CLIENT_PID" 2>/dev/null || true
    wait "$SERVER_PID" "$CLIENT_PID" 2>/dev/null || true
    echo "[phase2] Done."
}
trap cleanup INT TERM

wait -n "$SERVER_PID" "$CLIENT_PID" 2>/dev/null || true
cleanup
