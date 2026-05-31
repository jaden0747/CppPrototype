# Using Conan Profiles

## What is a Conan profile
A Conan profile is a named configuration that captures build settings, compiler options, environment variables and other configuration values used by Conan when resolving and building packages. Profiles make it easy to reproduce builds across machines and CI.

Conan profiles are typically stored under `conan/profiles/` in a project repository or in the global Conan configuration (`~/.conan/profiles/`). In this repo we keep project-specific profiles in `conan/profiles/` (e.g. `conan/profiles/windows`).

## Common profile commands
- List available profiles:

```bash
conan profile list
```

- Create a new profile (copy default or detect automatically):

```bash
conan profile new default --detect
# or copy an existing profile
conan profile new myprofile --base default
```

- Show profile contents:

```bash
conan profile show windows
# or show by file path
conan profile show conan/profiles/windows
```

- Update a profile setting:

```bash
conan profile update settings.compiler=msvc conan/profiles/windows
conan profile update settings.compiler.version=19.36 conan/profiles/windows
conan profile update env.CC=clang conan/profiles/linux
```

## Using a profile for install/create/build
Specify a profile by name or by path using `-pr` or `--profile`.

- Install dependencies into a build folder using a profile:

```bash
conan install . -pr=windows -if=build --build=missing
# or provide path to profile file
conan install . -pr=conan/profiles/windows -if=build --build=missing
```

- Create a package (local build) using a profile:

```bash
conan create . myuser/stable -pr=windows
```

- Export conan toolchain for CMake and use with a profile:

```bash
# generate files into build dir
conan install . -pr=windows -if=build --build=missing
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
cmake --build build
```

## Example contents (Windows profile)
A `conan/profiles/windows` file typically contains `settings`, `options`, and `env` sections. Example:

```
[settings]
os=Windows
os_build=Windows
arch=x86_64
arch_build=x86_64
compiler=msvc
compiler.version=19.36
compiler.runtime=MD
build_type=Release

[options]
# project-specific package options

[env]
CC=cl
CXX=cl

[build_requires]
# optional build-require packages
```

Adjust `compiler.version`, `compiler.runtime` and other settings to match your toolchain.

## Best practices
- Store reproducible project profiles under `conan/profiles/` and commit them.
- Use descriptive profile names (e.g. `windows-msvc-19.36-release`, `linux-clang-16-debug`).
- Pin exact compiler versions in CI to ensure reproducible builds.
- Use `conan lock` files for deterministic dependency resolution in CI.

## CI tips
- On CI, install Conan then copy the repository `conan/profiles/` to the runner and call `conan install` with `-pr` to select the repo profile.
- Use `--build=missing` in development pipelines; prefer prebuilt packages in release pipelines.

## Troubleshooting
- If Conan cannot find the profile name, pass the file path to `-pr` (e.g. `-pr=conan/profiles/windows`).
- Use `conan profile show <name>` to verify the settings that Conan will use.
- If builds fail due to compiler mismatch, verify `compiler` and `compiler.version` match the actual compiler on the machine.

## Helpful commands recap
```bash
conan profile list
conan profile show windows
conan profile new default --detect
conan profile update settings.compiler=msvc conan/profiles/windows
conan install . -pr=conan/profiles/windows -if=build --build=missing
conan create . myuser/stable -pr=windows
```

---

File location suggestions:
- Project profiles: `conan/profiles/*`
- Global profiles: `%USERPROFILE%/.conan/profiles/` (Windows) or `~/.conan/profiles/` (Unix)

If you'd like, I can add an example `windows` profile tailored to your current toolchain and Visual Studio version — tell me which compiler/runtime you use (MSVC version / Visual Studio version or clang/msvc-clang-cl).
