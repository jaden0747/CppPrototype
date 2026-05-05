# Dot-source this file to load the dev environment into the current shell:
#   . .\scripts\build\dev-env.ps1
#
# It does two things:
#   1. Activates the conda 'tool' env (where conan + ninja live)
#   2. Imports VS 2022 Professional MSVC x64 environment so that clang-cl
#      can find the MSVC CRT, Windows SDK headers, and link.exe.

$ErrorActionPreference = 'Stop'

# --- 1. Activate conda 'tool' env -------------------------------------------
if (-not (Get-Command conda -ErrorAction SilentlyContinue)) {
    throw "conda not found on PATH. Install Anaconda/Miniconda and re-run."
}

if ($env:CONDA_DEFAULT_ENV -ne 'tool') {
    Write-Host "[dev-env] Activating conda env 'tool'..." -ForegroundColor Cyan
    conda activate tool
    if ($LASTEXITCODE -ne 0) { throw "conda activate tool failed" }
}

# --- 2. Load VS 2022 MSVC x64 environment -----------------------------------
$vcvars = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat not found at $vcvars"
}

if (-not $env:VSCMD_VER) {
    Write-Host "[dev-env] Loading VS 2022 MSVC x64 environment..." -ForegroundColor Cyan
    $output = cmd /c "`"$vcvars`" >nul 2>&1 && set"
    foreach ($line in $output) {
        if ($line -match '^([^=]+)=(.*)$') {
            Set-Item -Path "env:$($matches[1])" -Value $matches[2]
        }
    }
}

# --- 3. Verify clang-cl is reachable ----------------------------------------
if (-not (Get-Command clang-cl -ErrorAction SilentlyContinue)) {
    Write-Warning "clang-cl not on PATH. Expected at C:\Program Files\LLVM\bin"
} else {
    Write-Host "[dev-env] Ready. clang-cl: $((Get-Command clang-cl).Source)" -ForegroundColor Green
}
