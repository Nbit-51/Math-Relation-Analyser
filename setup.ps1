# Full setup: Emscripten SDK + WASM build
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

Write-Host '=== Math Relation Analyser — Setup ===' -ForegroundColor Cyan

$emsdkRoot = Join-Path $env:USERPROFILE 'emsdk'

if (-not (Test-Path $emsdkRoot)) {
    Write-Host 'Cloning Emscripten SDK...' -ForegroundColor Yellow
    git clone https://github.com/emscripten-core/emsdk.git $emsdkRoot
}

Push-Location $emsdkRoot
Write-Host 'Installing Emscripten (this may take several minutes)...' -ForegroundColor Yellow
.\emsdk install latest
.\emsdk activate latest
Pop-Location

Write-Host 'Building project...' -ForegroundColor Cyan
& "$PSScriptRoot\build-wasm.ps1"

Write-Host ''
Write-Host 'Setup complete. Start the app with:' -ForegroundColor Green
Write-Host '  npm start' -ForegroundColor White
Write-Host 'Then open http://localhost:3000' -ForegroundColor White
