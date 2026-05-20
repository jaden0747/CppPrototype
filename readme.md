# Prototype

Minimal C++ application framework with three core features:

1. **Settings System** — JSON-backed runtime configuration with typed accessors and change tracking
2. **Sender/Receiver Ports** — Zero-copy multithreaded data transfer via ring-buffer mempool
3. **Debug Window** — ImGui over OpenGL 3.3 + GLFW for runtime inspection

## Build

```bash
# Install dependencies
conan install . --output-folder=build/Debug --build=missing -s build_type=Debug

# Configure & build
cmake --preset conan-default
cmake --build build/Debug
```

## Targets

| Target | Description |
|--------|-------------|
| `app` | Main window with ImGui panels showing settings + data ports |
| `cli_server` | Telnet CLI for inspecting/modifying settings at runtime |
| `tests` | GTest unit tests for data_container and settings |

## Project Structure

```
include/
  settings/       — SettingsRegistry, SettingsItem, DirtyTracker, CommandRegistry
  mylib/          — data_container.hpp (Mempool, SenderPort, ReceiverPort)
app/
  main.cpp        — ImGui application entry point
  cli_server.cpp  — CLI/Telnet settings server
test/
  test_data_container.cpp
  test_settings.cpp
external/
  imgui/          — Dear ImGui (submodule)
resources/
  settings.json   — Default settings file
```
