#!/bin/bash

echo "🔧 Installation des dépendances BioMining pour Cloud Shell..."

# Mise à jour des packages
echo "📦 Mise à jour de la liste des packages..."
sudo apt-get update -qq

# Installation des dépendances de base
echo "🛠️ Installation des outils de développement..."
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    curl \
    wget \
    unzip

# Installation de Qt6
echo "🎨 Installation de Qt6..."
sudo apt-get install -y \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-charts-dev \
    libqt6test6 \
    qt6-l10n-tools \
    qt6-documentation-tools

# Installation d'OpenSSL
echo "🔐 Installation d'OpenSSL..."
sudo apt-get install -y \
    libssl-dev \
    libcrypto++-dev

# Installation de Boost (optionnel mais recommandé)
echo "🚀 Installation de Boost..."
sudo apt-get install -y \
    libboost-all-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    libboost-thread-dev

# Installation d'outils supplémentaires pour le développement
echo "🔧 Installation d'outils supplémentaires..."
sudo apt-get install -y \
    valgrind \
    gdb \
    clang \
    clang-format \
    doxygen \
    graphviz

# Vérification des versions installées
echo ""
echo "✅ Vérification des installations..."
echo "📋 Versions installées :"
echo "  - GCC: $(gcc --version | head -1)"
echo "  - CMake: $(cmake --version | head -1)"
echo "  - Qt: $(qmake6 --version | head -1 2>/dev/null || echo 'Qt6 installé')"
echo "  - OpenSSL: $(openssl version)"

# Vérification que tous les packages critiques sont installés
REQUIRED_PACKAGES=(
    "build-essential"
    "cmake" 
    "qt6-base-dev"
    "libssl-dev"
)

echo ""
echo "🔍 Vérification des packages critiques..."
ALL_OK=true

for package in "${REQUIRED_PACKAGES[@]}"; do
    if dpkg -l | grep -q "^ii  $package "; then
        echo "  ✅ $package"
    else
        echo "  ❌ $package - MANQUANT"
        ALL_OK=false
    fi
done

if [ "$ALL_OK" = true ]; then
    echo ""
    echo "🎉 Toutes les dépendances sont installées avec succès!"
    echo ""
    echo "📋 Prochaines étapes :"
    echo "  1. mkdir -p build && cd build"
    echo "  2. cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON"
    echo "  3. make -j\$(nproc)"
    echo ""
else
    echo ""
    echo "⚠️  Certaines dépendances sont manquantes. Essayez de les installer manuellement :"
    echo "  sudo apt-get install -y build-essential cmake qt6-base-dev libssl-dev"
fi

# Créer un répertoire build s'il n'existe pas
if [ ! -d "build" ]; then
    mkdir -p build
    echo "📁 Répertoire 'build' créé"
fi

echo ""
echo "✨ Script d'installation terminé!"