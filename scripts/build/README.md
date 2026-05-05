# Build Scripts (Ninja-based, cross-platform)

This folder contains the platform-specific helpers that drive a **Ninja-based**
build of the project so that CMake produces `compile_commands.json` (which the
Visual Studio generator does not support).

## Layout

```
conan/profiles/
    windows                # base   : clang-cl 19  + Ninja  (VS 2022 bundled LLVM)
    windows-debug          # +Debug   build_type, runtime_type=Debug   (MDd CRT)
    windows-release        # +Release build_type, runtime_type=Release (MD  CRT)

    linux                  # base   : gcc 11       + Ninja + libstdc++11
    linux-debug            # +Debug   build_type
    linux-release          # +Release build_type

    macos                  # base   : apple-clang 15 + Ninja + libc++
    macos-debug            # +Debug   build_type
    macos-release          # +Release build_type

scripts/build/
    dev-env.ps1            # (Windows) load conda 'tool' env + VS 2022 vcvars64
    configure.ps1          # (Windows) wraps conan install + cmake --preset
    configure.sh           # (Linux / macOS) same flow, picks profile via uname
```

The per-config profiles use Conan's `include()` directive to inherit from
their base profile. So `windows-debug` is only the lines that differ from
`windows`. Edit the base profile (`windows`, `linux`, `macos`) when you want
the change to apply to both Debug and Release.

## Per-platform prerequisites

| Platform | Required tools                                                                |
|----------|-------------------------------------------------------------------------------|
| Windows  | conda env `tool` containing `conan>=2.0` and `ninja`; VS 2022 Professional with the C++ workload (provides `vcvars64.bat`, the MSVC CRT, the Windows SDK, and a bundled clang-cl 19) |
| Linux    | `conan` (>=2.0), `cmake` (>=3.23), `ninja`, `gcc` and `g++` (matching `compiler.version` in `conan/profiles/linux`) |
| macOS    | `conan` (>=2.0), `cmake` (>=3.23), `ninja`, Xcode command-line tools (`xcode-select --install`) |

If your installed compiler version differs from the one in the profile (e.g.
GCC 13 instead of 11), bump `compiler.version` in the profile — Conan uses it
to compute package IDs and will rebuild dependencies from source when it
doesn't match.

## Quick start

### Windows (PowerShell)

```powershell
# Clean configure for Debug, then build.
.\scripts\build\configure.ps1 -Clean -Config Debug -Build

# Just regenerate Release build files (no clean, no build).
.\scripts\build\configure.ps1 -Config Release
```

### Linux / macOS (Bash)

```bash
chmod +x scripts/build/configure.sh        # first time only

# Clean configure for Debug, then build.
./scripts/build/configure.sh --clean --config Debug --build

# Override auto-detection (e.g. to use a custom profile).
./scripts/build/configure.sh --profile my-clang-linux --config Release
```

After a successful configure, `compile_commands.json` is mirrored into the
repository root (symlinked on POSIX, copied on Windows) so clangd / VSCode
IntelliSense pick it up.

## Conan profiles

The profiles in `conan/profiles/` are committed to the repo so the build is
reproducible across machines. Each script picks one based on host OS and
the requested config:

| Script          | Config       | Profile picked     | Override flag    |
|-----------------|--------------|--------------------|------------------|
| `configure.ps1` | `Debug`      | `windows-debug`    | (edit script)    |
| `configure.ps1` | `Release` *  | `windows-release`  | (edit script)    |
| `configure.sh`  | `Debug`      | `linux-debug` / `macos-debug`     | `--profile NAME` |
| `configure.sh`  | `Release` *  | `linux-release` / `macos-release` | `--profile NAME` |

\* `RelWithDebInfo` and `MinSizeRel` reuse the `-release` profile (release
   CRT on Windows) and the script overrides `build_type` via Conan's
   `--settings=build_type=...` flag.

### Tweaking a profile

Open the file under `conan/profiles/` and adjust:

- `compiler.version`        — bump to match your installed toolchain
- `compiler.runtime_version` *(Windows only)* — `v144` for VS 2022 17.10+, `v143` for 17.0-17.9
- `compiler.libcxx`         *(Linux/macOS)* — `libstdc++11` (GCC) or `libc++` (Clang)
- `arch`                    — set to `x86_64` on Intel Macs (default `armv8` is Apple Silicon)
- `build_type`              — `Release`, `Debug`, etc. (the script overrides this with `--config`)

Conan rebuilds dependencies from source whenever the profile produces a new
package ID, so unusual settings (older GCC, atypical libcxx) may trigger a
larger build the first time.

## Platform notes

### Windows: POSIX IPC excluded

The Windows build automatically excludes the POSIX IPC subsystem because it
relies on APIs (`mkfifo`, `O_NONBLOCK`, `SIGPIPE`, `EPIPE`/`EAGAIN`, `ssize_t`)
that map to a different Windows API family (`CreateNamedPipe`, `GetLastError`)
and would change the educational content of the lectures:

- `src/mylib/ipc/*.cpp`           — excluded by `src/mylib/CMakeLists.txt`
- `src/mylib/customscene.cpp`     — excluded (depends on the IPC channel)
- `test/test_ipc.cpp`             — excluded by the root `CMakeLists.txt`
- `Main::run()` in `mylib.cpp`    — guarded with `#if defined(_WIN32)`

To build / run / test the IPC code, use Linux, WSL, or macOS. The Windows host
is fine for editing, IntelliSense (via `compile_commands.json`), and building
non-IPC targets such as `settings_demo` and `cli_server`.

### Linux / macOS: full build available

The IPC tutorials (`mkfifo` / `shm_open` / `sem_open`) build natively. The
`Application` target runs the FIFO logger demo via `CustomScene`.
