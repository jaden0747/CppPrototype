#!/usr/bin/env bash
# Cross-platform configure flow: conan install + cmake configure with Ninja.
# Auto-picks the Conan profile from `uname -s`.
#
# Usage:
#   ./scripts/build/configure.sh                       # Debug, no clean
#   ./scripts/build/configure.sh --config Release
#   ./scripts/build/configure.sh --clean               # wipe build/ first
#   ./scripts/build/configure.sh --build               # also build after configuring
#   ./scripts/build/configure.sh --profile clang-linux # override auto-detection
#
# On Windows, use scripts/build/configure.ps1 instead — it loads the VS 2022
# environment that clang-cl needs.

set -euo pipefail

CONFIG="Debug"
CLEAN=0
BUILD=0
PROFILE_OVERRIDE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--config)   CONFIG="$2"; shift 2 ;;
        -p|--profile)  PROFILE_OVERRIDE="$2"; shift 2 ;;
        --clean)       CLEAN=1; shift ;;
        --build)       BUILD=1; shift ;;
        -h|--help)
            sed -n '2,12p' "$0"
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 2
            ;;
    esac
done

case "$CONFIG" in
    Debug|Release|RelWithDebInfo|MinSizeRel) ;;
    *) echo "Invalid --config: $CONFIG" >&2; exit 2 ;;
esac

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$REPO_ROOT"

# --- Pick the Conan profile based on host OS + config -----------------------
# Profiles are layered:  <platform>      = base toolchain
#                        <platform>-debug / <platform>-release  = build_type variants
# Debug picks the -debug variant; everything else (Release / RelWithDebInfo /
# MinSizeRel) uses the -release variant and overrides build_type via -s below.
if [[ "$CONFIG" == "Debug" ]]; then
    VARIANT="debug"
else
    VARIANT="release"
fi

if [[ -n "$PROFILE_OVERRIDE" ]]; then
    PROFILE_NAME="$PROFILE_OVERRIDE"
else
    case "$(uname -s)" in
        Linux*)   PROFILE_NAME="linux-$VARIANT" ;;
        Darwin*)  PROFILE_NAME="macos-$VARIANT" ;;
        MINGW*|MSYS*|CYGWIN*)
            echo "On Windows, run scripts/build/configure.ps1 (not this script)." >&2
            exit 2
            ;;
        *)
            echo "Unsupported OS: $(uname -s). Use --profile to specify manually." >&2
            exit 2
            ;;
    esac
fi

CONAN_PROFILE="$REPO_ROOT/conan/profiles/$PROFILE_NAME"
if [[ ! -f "$CONAN_PROFILE" ]]; then
    echo "Conan profile not found: $CONAN_PROFILE" >&2
    exit 1
fi

# --- Tool availability checks -----------------------------------------------
for tool in conan cmake ninja; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "[configure] '$tool' not found on PATH. Install it first." >&2
        exit 1
    fi
done

# --- Optional clean ----------------------------------------------------------
if [[ $CLEAN -eq 1 && -d "$REPO_ROOT/build" ]]; then
    echo "[configure] Removing existing build/ ..."
    rm -rf "$REPO_ROOT/build"
fi

# --- Conan install -----------------------------------------------------------
echo "[configure] Using profile: $PROFILE_NAME (host=$CONAN_PROFILE)"
echo "[configure] conan install ($CONFIG) ..."
conan install . \
    --profile:host="$CONAN_PROFILE" \
    --profile:build="$CONAN_PROFILE" \
    --settings=build_type="$CONFIG" \
    --build=missing

# --- CMake configure ---------------------------------------------------------
PRESET_NAME="conan-$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')"
echo "[configure] cmake --preset $PRESET_NAME ..."
cmake --preset "$PRESET_NAME" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# --- Mirror compile_commands.json to repo root ------------------------------
BUILD_DIR="$REPO_ROOT/build/$CONFIG"
COMPILE_DB="$BUILD_DIR/compile_commands.json"
if [[ -f "$COMPILE_DB" ]]; then
    if ln -sf "$COMPILE_DB" "$REPO_ROOT/compile_commands.json" 2>/dev/null; then
        echo "[configure] compile_commands.json symlinked to repo root."
    else
        cp -f "$COMPILE_DB" "$REPO_ROOT/compile_commands.json"
        echo "[configure] compile_commands.json copied to repo root."
    fi
else
    echo "[configure] WARNING: compile_commands.json not produced at $COMPILE_DB" >&2
fi

# --- Optional build ----------------------------------------------------------
if [[ $BUILD -eq 1 ]]; then
    echo "[configure] cmake --build --preset $PRESET_NAME ..."
    cmake --build --preset "$PRESET_NAME"
fi

echo "[configure] Done."
