#!/bin/bash
# BioMining Platform - Déploiement Rapide Cloud Shell
# Script de déploiement simplifié sans dépendances Qt6

set -e

echo "🧬 === BIOMINING PLATFORM - DÉPLOIEMENT RAPIDE ==="
echo "🚀 Version Cloud Shell Optimisée"
echo "✅ Sans Qt6SerialPort - Interface Web Pure"
echo "================================================="

# Configuration
DEPLOY_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$DEPLOY_DIR")"
DEMO_SCRIPT="$DEPLOY_DIR/simple_biomining_demo.py"

echo "📁 Répertoire déploiement: $DEPLOY_DIR"
echo "📁 Répertoire racine: $ROOT_DIR"

# Vérifications
if [ ! -f "$DEMO_SCRIPT" ]; then
    echo "❌ Script principal non trouvé: $DEMO_SCRIPT"
    exit 1
fi

echo "✅ Script principal: $DEMO_SCRIPT"

# Vérifier Python et dépendances
echo ""
echo "🐍 Vérification Python..."

python3 --version || {
    echo "❌ Python 3 non disponible"
    exit 1
}

# Installer Flask si nécessaire
python3 -c "import flask; print('✅ Flask disponible')" 2>/dev/null || {
    echo "📦 Installation Flask..."
    pip3 install flask
}

echo "✅ Toutes les dépendances disponibles"

# Arrêter les processus existants sur les ports courants
echo ""
echo "🧹 Nettoyage des processus existants..."

# Tuer les processus sur les ports 5000-5010
for port in {5000..5010}; do
    PID=$(lsof -ti tcp:$port 2>/dev/null || true)
    if [ ! -z "$PID" ]; then
        echo "🔄 Arrêt processus sur port $port (PID: $PID)"
        kill $PID 2>/dev/null || true
        sleep 0.5
    fi
done

echo "✅ Nettoyage terminé"

# Trouver un port disponible
echo ""
echo "🔍 Recherche port disponible..."

PORT=5000
while lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; do
    PORT=$((PORT + 1))
    if [ $PORT -gt 5020 ]; then
        echo "❌ Aucun port disponible dans la plage 5000-5020"
        exit 1
    fi
done

echo "✅ Port sélectionné: $PORT"

# Variables d'environnement
export PORT=$PORT
export HOST="0.0.0.0"
export FLASK_ENV="production"

# Fonction de nettoyage
cleanup() {
    echo ""
    echo "🧹 Nettoyage final..."
    PID=$(lsof -ti tcp:$PORT 2>/dev/null || true)
    if [ ! -z "$PID" ]; then
        kill $PID 2>/dev/null || true
    fi
    echo "✅ Nettoyage terminé"
}

trap cleanup EXIT INT TERM

# Lancement du serveur
echo ""
echo "🚀 Lancement BioMining Platform..."
echo "📊 Interface Web Complete: MEA + Bitcoin Mining"
echo "🌐 URL: http://localhost:$PORT"
echo "⏱️  Démarrage: $(date)"
echo ""
echo "🔗 IMPORTANT: Utilisez Web Preview dans Cloud Shell"
echo "   1. Cliquez sur l'icône 'Web Preview' (🔗)"  
echo "   2. Sélectionnez 'Preview on port $PORT'"
echo "   3. Ou utilisez 'Change Port' pour $PORT"
echo ""
echo "⚡ Fonctionnalités disponibles:"
echo "   ✅ Interface MEA (60 électrodes simulées)"
echo "   ✅ Mining Bitcoin (avec validation Genesis)"
echo "   ✅ Monitoring temps réel"
echo "   ✅ Sans erreurs Qt6SerialPort"
echo ""
echo "Press Ctrl+C pour arrêter"
echo "================================================="

# Lancer le serveur
cd "$DEPLOY_DIR"
python3 simple_biomining_demo.py