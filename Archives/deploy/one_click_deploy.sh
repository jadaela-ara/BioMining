#!/bin/bash
# BioMining Platform - Déploiement Ultra-Rapide (One-Click)
# Usage: curl -sSL https://raw.githubusercontent.com/jadaela-ara/BioMining/main/deploy/one_click_deploy.sh | bash

set -e

echo "🧬 === BIOMINING PLATFORM - ONE-CLICK DEPLOY ==="
echo "⚡ Ultra-Fast Cloud Shell Deployment"
echo "✅ No Qt6, No Compilation, Pure Web Interface"
echo "==============================================="

# Vérifier si nous sommes déjà dans le bon répertoire
if [ -f "deploy/simple_biomining_demo.py" ]; then
    echo "✅ BioMining repository already cloned"
    cd "$(pwd)"
else
    echo "📥 Cloning BioMining repository..."
    if command -v git >/dev/null 2>&1; then
        git clone https://github.com/jadaela-ara/BioMining.git
        cd BioMining
    else
        echo "❌ Git not found. Please install git first."
        exit 1
    fi
fi

echo "✅ Repository ready"

# Vérifier Python
if ! command -v python3 >/dev/null 2>&1; then
    echo "❌ Python 3 not found. Please install Python 3 first."
    exit 1
fi

echo "✅ Python 3 available"

# Installer Flask si nécessaire
if ! python3 -c "import flask" >/dev/null 2>&1; then
    echo "📦 Installing Flask..."
    pip3 install flask --user
fi

echo "✅ Flask available"

# Trouver un port libre
PORT=5000
while lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; do
    PORT=$((PORT + 1))
done

echo "🔍 Using port: $PORT"

# Nettoyer les processus existants
pkill -f "simple_biomining_demo.py" 2>/dev/null || true

echo ""
echo "🚀 Starting BioMining Platform..."
echo "🌐 Interface will be available at:"
echo "   Local: http://localhost:$PORT"
echo "   Cloud Shell: Use Web Preview on port $PORT"
echo ""
echo "📊 Features:"
echo "   ✅ MEA Interface (60 electrodes)"
echo "   ✅ Bitcoin Mining (with Genesis validation)" 
echo "   ✅ Real-time monitoring"
echo "   ✅ No Qt6 dependencies"
echo ""
echo "🔗 IMPORTANT for Cloud Shell users:"
echo "   1. Look for the Web Preview button (🔗) in Cloud Shell"
echo "   2. Select 'Preview on port $PORT'"
echo "   3. Complete interface will open in new tab"
echo ""
echo "Press Ctrl+C to stop the server"
echo "==============================================="

# Lancer le serveur
export PORT=$PORT
export HOST="0.0.0.0"
cd deploy
exec python3 simple_biomining_demo.py