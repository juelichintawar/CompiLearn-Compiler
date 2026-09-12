@echo off
title CompiLearn - Starting Compiler

REM ========================================
REM          COMPILEARN
REM      Portable Launcher
REM ========================================

REM Get the folder where this BAT file is located
set "PROJECT_DIR=%~dp0"

echo ========================================
echo          COMPILEARN
echo      Compiler Construction IDE
echo ========================================
echo.
echo Project location:
echo %PROJECT_DIR%
echo.
echo Starting MSYS2 UCRT64 backend...
echo.

start "CompiLearn Backend" "C:\msys64\usr\bin\bash.exe" -lc "cd '%PROJECT_DIR%' && source .venv/bin/activate && python backend/app.py"

echo Waiting for Flask backend to start...
echo.

:CHECK_SERVER
powershell -NoProfile -Command "try { $r=Invoke-WebRequest -Uri 'http://127.0.0.1:5000/api/health' -UseBasicParsing -TimeoutSec 1; if ($r.StatusCode -eq 200) { exit 0 } else { exit 1 } } catch { exit 1 }"

if errorlevel 1 (
    timeout /t 1 /nobreak >nul
    goto CHECK_SERVER
)

echo.
echo ========================================
echo      COMPILEARN IS READY!
echo ========================================
echo.
echo Opening CompiLearn in your browser...
echo.

start "" "http://127.0.0.1:5000"

echo.
echo Keep the backend window open while using
echo CompiLearn.
echo.
pause