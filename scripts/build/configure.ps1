# Full configure flow: conan install + cmake configure with Ninja + clang-cl.
#
# Usage:
#   .\scripts\build\configure.ps1                    # Debug, no clean
#   .\scripts\build\configure.ps1 -Config Release
#   .\scripts\build\configure.ps1 -Clean             # wipe build/ first
#   .\scripts\build\configure.ps1 -Build             # also build after configuring

[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Config = 'Debug',

    [switch]$Clean,
    [switch]$Build
)

$ErrorActionPreference = 'Stop'

$RepoRoot = Resolve-Path "$PSScriptRoot\..\.."
Set-Location $RepoRoot

. "$PSScriptRoot\dev-env.ps1"

if ($Clean -and (Test-Path "$RepoRoot\build")) {
    Write-Host "[configure] Removing existing build/ ..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "$RepoRoot\build"
}

# Pick the per-config Conan profile.
#   Debug  -> windows-debug  (build_type=Debug,   runtime_type=Debug   -> MDd CRT)
#   else   -> windows-release (build_type=Release, runtime_type=Release -> MD  CRT)
# RelWithDebInfo / MinSizeRel reuse the release profile (release CRT) and we
# pass `-s build_type=...` to override the actual optimization level.
if ($Config -eq 'Debug') {
    $ProfileName = 'windows-debug'
} else {
    $ProfileName = 'windows-release'
}
$ConanProfile = "$RepoRoot\conan\profiles\$ProfileName"
if (-not (Test-Path $ConanProfile)) {
    throw "Conan profile not found: $ConanProfile"
}

Write-Host "[configure] Using profile: $ProfileName" -ForegroundColor Cyan
Write-Host "[configure] conan install ($Config) ..." -ForegroundColor Cyan
conan install . `
    --profile:host=$ConanProfile `
    --profile:build=$ConanProfile `
    --settings=build_type=$Config `
    --build=missing
if ($LASTEXITCODE -ne 0) { throw "conan install failed" }

# Conan with cmake_layout + single-config Ninja places the preset under
# build\<Config>\generators\CMakePresets.json and writes a top-level
# CMakeUserPresets.json that exposes presets named conan-<lowercase config>.
$PresetName = "conan-$($Config.ToLower())"

Write-Host "[configure] cmake --preset $PresetName ..." -ForegroundColor Cyan
cmake --preset $PresetName -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }

# Mirror compile_commands.json to the repo root so clangd / VSCode pick it up
# regardless of which build config you generated last.
$BuildDir = "$RepoRoot\build\$Config"
$CompileDb = "$BuildDir\compile_commands.json"
if (Test-Path $CompileDb) {
    Copy-Item -Force $CompileDb "$RepoRoot\compile_commands.json"
    Write-Host "[configure] compile_commands.json copied to repo root." -ForegroundColor Green
} else {
    Write-Warning "compile_commands.json not produced at $CompileDb"
}

if ($Build) {
    $BuildPreset = "conan-$($Config.ToLower())"
    Write-Host "[configure] cmake --build --preset $BuildPreset ..." -ForegroundColor Cyan
    cmake --build --preset $BuildPreset
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }
}

Write-Host "[configure] Done." -ForegroundColor Green
