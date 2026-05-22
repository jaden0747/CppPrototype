#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# start_stream.sh — Launch stream_server and stream_client side-by-side
#
# Usage:
#   ./script/start_stream.sh                  # both on localhost:9999
#   ./script/start_stream.sh --port 8888      # use a different port (NYI)
#
# The server window captures its own OpenGL framebuffer and sends raw RGB
# frames over TCP.  The client window receives them and displays via ImGui.
# Press Ctrl+C here (or close either window) to stop both processes.
# ---------------------------------------------------------------------------
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build/Debug"

# ── Locate binaries ────────────────────────────────────────────────────────
SERVER="$BUILD_DIR/stream_server"
CLIENT="$BUILD_DIR/stream_client"

if [[ ! -x "$SERVER" ]]; then
    echo "ERROR: stream_server not found at $SERVER"
    echo "Build the project first:"
    echo "  cmake --preset conan-debug && cmake --build build/Debug --target stream_server stream_client"
    exit 1
fi
if [[ ! -x "$CLIENT" ]]; then
    echo "ERROR: stream_client not found at $CLIENT"
    echo "Build the project first (same command above)."
    exit 1
fi

# ── Start server ───────────────────────────────────────────────────────────
echo "[start_stream] Starting stream_server …"
cd "$BUILD_DIR"
./stream_server &
SERVER_PID=$!

# Give the server 1 second to open its TCP socket before the client connects
sleep 1

# ── Start client ───────────────────────────────────────────────────────────
echo "[start_stream] Starting stream_client …"
./stream_client &
CLIENT_PID=$!

echo ""
echo "[start_stream] Both running."
echo "  Server PID : $SERVER_PID"
echo "  Client PID : $CLIENT_PID"
echo ""
echo "  Close either window or press Ctrl+C here to stop both."
echo ""

# ── Wait and clean up ──────────────────────────────────────────────────────
cleanup() {
    echo ""
    echo "[start_stream] Stopping …"
    kill "$SERVER_PID" "$CLIENT_PID" 2>/dev/null || true
    wait "$SERVER_PID" "$CLIENT_PID" 2>/dev/null || true
    echo "[start_stream] Done."
}
trap cleanup INT TERM

# Wait for whichever process exits first, then kill the other
wait -n "$SERVER_PID" "$CLIENT_PID" 2>/dev/null || true
cleanup
