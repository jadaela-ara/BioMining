#!/bin/bash
# Script to run BioMining Web Interface (Alternative to Qt GUI)

set -e

echo "🌐 BioMining Web Interface Runner"
echo "=================================="

# Check if Python demo exists
DEMO_SCRIPT="./real_mea_demo.py"
if [ ! -f "$DEMO_SCRIPT" ]; then
    echo "❌ Error: Web demo script not found at $DEMO_SCRIPT"
    exit 1
fi

echo "✅ Found web interface: $DEMO_SCRIPT"
echo ""

# Get available port
PORT=8080
while lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; do
    PORT=$((PORT + 1))
done

echo "🚀 Starting BioMining Web Interface on port $PORT..."
echo ""
echo "📱 Access the interface at:"
echo "   Local:  http://localhost:$PORT"
echo "   Cloud Shell Web Preview: Use the web preview feature"
echo ""
echo "🔧 Features available in web interface:"
echo "   ✅ MEA device configuration"
echo "   ✅ Real-time signal monitoring" 
echo "   ✅ Electrode impedance testing"
echo "   ✅ Data acquisition controls"
echo "   ✅ Device status monitoring"
echo ""
echo "Use Ctrl+C to stop the web server"
echo "======================================"
echo ""

# Run the web interface
cd /home/user/webapp
python3 "$DEMO_SCRIPT" --port $PORT --host 0.0.0.0