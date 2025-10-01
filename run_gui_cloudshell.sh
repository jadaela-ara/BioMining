#!/bin/bash
# Script to run BioMining GUI in Google Cloud Shell using virtual display

set -e

echo "🖥️  BioMining GUI Cloud Shell Runner"
echo "====================================="

# Check if GUI executable exists
GUI_EXEC="./bin/biomining_gui"
if [ ! -f "$GUI_EXEC" ]; then
    echo "❌ Error: GUI executable not found at $GUI_EXEC"
    echo "Please build the project first with:"
    echo "   cd /home/user/webapp && mkdir -p build && cd build"
    echo "   cmake .. && make -j$(nproc)"
    exit 1
fi

echo "✅ Found GUI executable: $GUI_EXEC"

# Set up virtual display
export DISPLAY=:99
XVFB_PID=""

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "🧹 Cleaning up..."
    if [ ! -z "$XVFB_PID" ]; then
        kill $XVFB_PID 2>/dev/null || true
        echo "✅ Stopped virtual display"
    fi
}

# Set trap for cleanup
trap cleanup EXIT INT TERM

echo "🚀 Starting virtual display (Xvfb)..."
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp &
XVFB_PID=$!

# Wait for display to start
sleep 2

# Check if Xvfb is running
if ! kill -0 $XVFB_PID 2>/dev/null; then
    echo "❌ Failed to start virtual display"
    exit 1
fi

echo "✅ Virtual display started (PID: $XVFB_PID)"
echo "🎯 Display: $DISPLAY (1024x768x24)"

echo ""
echo "🔄 Starting BioMining GUI..."
echo "Note: GUI will run in virtual display (no visual output in Cloud Shell)"
echo "Use Ctrl+C to stop the application"
echo ""

# Run the GUI application
cd /home/user/webapp
$GUI_EXEC "$@"
