# Dot-source this file to load the dev environment into the current shell:
#   . .\scripts\build\dev-env.ps1
#
# It does three things:
#   1. Adds the conda 'cpp' env (where conan, cmake, ninja live) to PATH
#   2. Imports VS 2025 Community MSVC x64 environment so that clang-cl
#      can find the MSVC CRT, Windows SDK headers, and link.exe.
#   3. Adds the VS-bundled LLVM bin dir to PATH so clang-cl is reachable by name

$ErrorActionPreference = 'Stop'

# --- 1. Add conda 'cpp' env to PATH -----------------------------------------
$CppEnvScripts = "C:\Users\jaden\anaconda3\envs\cpp\Scripts"
$CppEnvRoot    = "C:\Users\jaden\anaconda3\envs\cpp"
if (-not (Test-Path $CppEnvScripts)) {
    throw "conda 'cpp' env not found at $CppEnvScripts. Create it: conda create -n cpp python && conda activate cpp && pip install conan ninja cmake"
}
if ($env:PATH -notlike "*envs\cpp\Scripts*") {
    $env:PATH = "$CppEnvScripts;$CppEnvRoot;$env:PATH"
    Write-Host "[dev-env] Added conda cpp env to PATH." -ForegroundColor Green
}

# --- 2. Load VS 2025 Community MSVC x64 environment -------------------------
$vcvars = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat not found at $vcvars. Install VS 2025 Community with C++ workload."
}

if (-not $env:VSCMD_VER) {
    Write-Host "[dev-env] Loading VS 2025 Community MSVC x64 environment..." -ForegroundColor Cyan
    $output = cmd /c "`"$vcvars`" >nul 2>&1 && set"
    foreach ($line in $output) {
        if ($line -match '^([^=]+)=(.*)$') {
            Set-Item -Path "env:$($matches[1])" -Value $matches[2]
        }
    }
}

# --- 3. Add VS-bundled LLVM bin to PATH so clang-cl is reachable by name ----
$LlvmBin = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin"
if ($env:PATH -notlike "*Llvm\x64\bin*") {
    $env:PATH = "$LlvmBin;$env:PATH"
}

# --- 4. Verify clang-cl is reachable ----------------------------------------
if (-not (Get-Command clang-cl -ErrorAction SilentlyContinue)) {
    Write-Warning "clang-cl not on PATH. Expected at $LlvmBin"
} else {
    Write-Host "[dev-env] Ready. clang-cl: $((Get-Command clang-cl).Source)" -ForegroundColor Green
}
