@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0build-wasm.ps1"
exit /b %ERRORLEVEL%
