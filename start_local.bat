@echo off
REM ================================================================
REM BioMining Platform - Local Deployment Script (Windows)
REM ================================================================
REM This script starts the BioMining platform locally
REM No Docker, no Cloud Run, no costs!
REM ================================================================

echo ================================================================
echo 🚀 BioMining Platform - Local Deployment
echo ================================================================
echo.

REM Check if we're in the right directory
if not exist "requirements.txt" (
    echo ❌ Error: requirements.txt not found
    echo Please run this script from the BioMining root directory
    pause
    exit /b 1
)

REM Check Python
echo 📋 Checking Python version...
python --version
if errorlevel 1 (
    echo ❌ Python not found! Please install Python 3.10+
    pause
    exit /b 1
)
echo ✅ Python found
echo.

REM Check if virtual environment exists
if not exist "venv" (
    echo ⚠️  Virtual environment not found
    echo 🔧 Creating virtual environment...
    python -m venv venv
    echo ✅ Virtual environment created
    echo.
)

REM Activate virtual environment
echo 🔧 Activating virtual environment...
call venv\Scripts\activate.bat
echo ✅ Virtual environment activated
echo.

REM Install dependencies
echo 📦 Installing dependencies...
pip install -q -r requirements.txt
echo ✅ Dependencies installed
echo.

REM Display information
echo ================================================================
echo 🎉 BioMining Platform Ready!
echo ================================================================
echo.
echo 📍 Access URLs:
echo    Local:    http://localhost:8000
echo    Network:  http://YOUR_LOCAL_IP:8000
echo.
echo 📊 API Endpoints:
echo    Health:   http://localhost:8000/api/health
echo    Training: http://localhost:8000/api/training/historical/status
echo.
echo 🛑 To stop the server: Press Ctrl + C
echo.
echo ================================================================
echo 🚀 Starting server...
echo ================================================================
echo.

REM Start the server
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --reload

pause
