#!/bin/bash

# 🚀 Setup Express pour BioMining sur Cloud Shell

echo "🚀 BioMining - Setup Express pour Cloud Shell"
echo "=============================================="

# Installation rapide des dépendances critiques
echo "📦 Installation des dépendances essentielles..."
sudo apt-get update -qq && sudo apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-charts-dev \
    libssl-dev \
    pkg-config

# Vérification rapide
echo "✅ Vérification..."
if command -v cmake >/dev/null && command -v qmake6 >/dev/null && command -v gcc >/dev/null; then
    echo "✅ Dépendances installées avec succès!"
    
    # Configuration et compilation
    echo "🏗️ Configuration et compilation..."
    mkdir -p build && cd build
    
    if cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON; then
        echo "✅ Configuration CMake réussie!"
        
        echo "🔨 Compilation en cours..."
        if make -j$(nproc); then
            echo "🎉 Compilation réussie!"
            echo ""
            echo "✅ BioMining est prêt à utiliser!"
            echo ""
            echo "📋 Commandes utiles :"
            echo "  - Tests: make test"
            echo "  - Documentation: make docs (si disponible)"
            echo "  - Nettoyage: make clean"
        else
            echo "❌ Erreur de compilation. Consultez CLOUDSHELL_SETUP.md pour le dépannage."
            exit 1
        fi
    else
        echo "❌ Erreur de configuration CMake. Vérifiez les dépendances."
        exit 1
    fi
else
    echo "❌ Installation des dépendances échouée. Essayez l'installation manuelle."
    echo "Consultez CLOUDSHELL_SETUP.md pour plus de détails."
    exit 1
fi