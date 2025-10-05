#!/bin/bash
# ================================================================
# HYBRID BITCOIN MINING PLATFORM - WEB INTERFACE LAUNCHER
# Advanced startup script for the complete web platform
# ================================================================

set -e  # Exit on any error

echo "🚀 Starting Hybrid Bitcoin Mining Platform Web Interface"
echo "================================================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} ✅ $1"
}

print_warning() {
    echo -e "${YELLOW}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} ⚠️ $1"
}

print_error() {
    echo -e "${RED}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} ❌ $1"
}

# Check if we're in the correct directory
if [ ! -f "api/server.py" ]; then
    print_error "Please run this script from the web directory"
    exit 1
fi

# Set environment variables
export PYTHONPATH="${PWD}/..:${PYTHONPATH}"
export PYTHONUNBUFFERED=1
export PORT=${PORT:-8080}
export HOST=${HOST:-0.0.0.0}
export WORKERS=${WORKERS:-4}
export LOG_LEVEL=${LOG_LEVEL:-info}

print_status "Environment configured:"
print_status "  - PYTHONPATH: ${PYTHONPATH}"
print_status "  - PORT: ${PORT}"
print_status "  - HOST: ${HOST}"
print_status "  - WORKERS: ${WORKERS}"
print_status "  - LOG_LEVEL: ${LOG_LEVEL}"

# Check Python version
python_version=$(python3 --version 2>&1 | cut -d' ' -f2)
print_status "Python version: ${python_version}"

# Create necessary directories
print_status "Creating necessary directories..."
mkdir -p uploads logs static/js static/css
print_success "Directories created"

# Check and install dependencies
print_status "Checking dependencies..."
if [ -f "requirements.txt" ]; then
    print_status "Installing/updating Python dependencies..."
    pip install -r requirements.txt
    print_success "Python dependencies installed"
else
    print_warning "requirements.txt not found, installing basic dependencies..."
    pip install fastapi uvicorn websockets python-multipart psutil
fi

# Check if C++ components are available
print_status "Checking C++ components..."
if [ -f "../src/cpp/hybrid_bitcoin_miner.cpp" ]; then
    print_success "C++ components found"
    
    # Check if compiled
    if [ ! -f "../build/hybrid_bitcoin_miner" ]; then
        print_warning "C++ components not compiled, building..."
        cd ..
        if [ -d "build" ]; then
            rm -rf build
        fi
        mkdir -p build
        cd build
        
        if command -v cmake >/dev/null 2>&1; then
            cmake ..
            make -j$(nproc)
            print_success "C++ components built successfully"
        else
            print_warning "CMake not found, C++ components will use mock implementations"
        fi
        cd ../web
    else
        print_success "C++ components already compiled"
    fi
else
    print_warning "C++ components not found, will use mock implementations"
fi

# Check if biological learning module is available
print_status "Checking biological learning module..."
if [ -f "../biological_bitcoin_learning.py" ]; then
    print_success "Biological learning module found"
else
    print_warning "Biological learning module not found"
fi

# Check if MEA interface is available
print_status "Checking MEA interface..."
if [ -f "../real_mea_interface.py" ]; then
    print_success "MEA interface found"
else
    print_warning "MEA interface not found"
fi

# Start the web server
print_status "Starting web server..."
print_status "Server will be available at: http://${HOST}:${PORT}"
print_status "API documentation at: http://${HOST}:${PORT}/docs"

# Function to handle cleanup on exit
cleanup() {
    print_status "Shutting down web server..."
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

# Check if running in production or development mode
if [ "$NODE_ENV" = "production" ]; then
    print_status "Starting in PRODUCTION mode"
    
    # Use Gunicorn for production
    if command -v gunicorn >/dev/null 2>&1; then
        gunicorn api.server:app \
            --bind ${HOST}:${PORT} \
            --workers ${WORKERS} \
            --worker-class uvicorn.workers.UvicornWorker \
            --timeout 300 \
            --keepalive 5 \
            --max-requests 1000 \
            --max-requests-jitter 100 \
            --log-level ${LOG_LEVEL} \
            --access-logfile logs/access.log \
            --error-logfile logs/error.log
    else
        print_warning "Gunicorn not found, falling back to Uvicorn"
        uvicorn api.server:app \
            --host ${HOST} \
            --port ${PORT} \
            --workers ${WORKERS} \
            --log-level ${LOG_LEVEL}
    fi
else
    print_status "Starting in DEVELOPMENT mode"
    
    # Use Uvicorn for development with auto-reload
    uvicorn api.server:app \
        --host ${HOST} \
        --port ${PORT} \
        --reload \
        --reload-dir . \
        --reload-dir .. \
        --log-level ${LOG_LEVEL} \
        --access-log
fi