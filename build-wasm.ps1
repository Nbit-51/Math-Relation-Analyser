# Build WASM module using Emscripten
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

$emsdkRoot = Join-Path $env:USERPROFILE 'emsdk'
$emsdkEnv  = Join-Path $emsdkRoot 'emsdk_env.ps1'

if (-not (Test-Path $emsdkEnv)) {
    Write-Host 'Emscripten not found. Run setup.ps1 first.' -ForegroundColor Red
    exit 1
}

. $emsdkEnv

New-Item -ItemType Directory -Force -Path wasm | Out-Null

Write-Host 'Building C++ WASM engine...' -ForegroundColor Cyan
emcc bridge.cpp -std=c++17 -O3 -fexceptions `
  -s WASM=1 `
  -s MODULARIZE=1 `
  -s EXPORT_NAME=createRelationEngine `
  '-s' 'EXPORTED_FUNCTIONS=["_analyse"]' `
  '-s' 'EXPORTED_RUNTIME_METHODS=["ccall","cwrap","UTF8ToString"]' `
  -s DISABLE_EXCEPTION_CATCHING=0 `
  -s ALLOW_MEMORY_GROWTH=1 `
  -o wasm/relation_engine.js

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host 'OK: wasm/relation_engine.js + relation_engine.wasm' -ForegroundColor Green

# Native CLI
Write-Host 'Building native CLI (math-analyser.exe)...' -ForegroundColor Cyan
g++ -std=c++17 -O2 main.cpp -o math-analyser.exe
if ($LASTEXITCODE -eq 0) { Write-Host 'OK: math-analyser.exe' -ForegroundColor Green }
