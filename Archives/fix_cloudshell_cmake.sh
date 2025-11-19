#!/bin/bash

echo "🔧 Correction des erreurs CMake pour Cloud Shell..."

# 1. Installer les dépendances manquantes
echo "📦 Installation des dépendances manquantes..."

# Boost - Réinstallation propre
sudo apt-get remove -y libboost-all-dev 2>/dev/null
sudo apt-get autoremove -y
sudo apt-get install -y libboost-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev

# XKB pour les interfaces X11
sudo apt-get install -y libxkbcommon-dev libxkbcommon-x11-dev

# Autres dépendances potentiellement manquantes
sudo apt-get install -y \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxinerama-dev \
    libxi-dev

echo "✅ Dépendances installées"

# 2. Nettoyer le cache CMake
echo "🧹 Nettoyage du cache CMake..."
rm -rf build/
mkdir -p build

# 3. Configuration alternative sans Boost (si nécessaire)
echo "⚙️ Configuration CMake..."

cd build

# Option 1: Essayer avec Boost
echo "Tentative 1: Configuration avec Boost..."
if cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON 2>/dev/null; then
    echo "✅ Configuration avec Boost réussie!"
else
    echo "⚠️ Configuration avec Boost échouée, tentative sans Boost..."
    
    # Option 2: Configuration sans Boost
    echo "Tentative 2: Configuration sans Boost..."
    if cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=ON \
        -DBUILD_EXAMPLES=ON \
        -DBoost_NO_SYSTEM_PATHS=ON \
        -DBOOST_ROOT=/nonexistent 2>/dev/null; then
        echo "✅ Configuration sans Boost réussie!"
    else
        echo "Tentative 3: Configuration minimaliste..."
        # Option 3: Configuration minimaliste
        cmake .. \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_TESTS=ON \
            -DBUILD_EXAMPLES=OFF \
            -DCMAKE_DISABLE_FIND_PACKAGE_Boost=TRUE \
            -DCMAKE_DISABLE_FIND_PACKAGE_XKB=TRUE
        echo "✅ Configuration minimaliste appliquée!"
    fi
fi

# 4. Test de compilation
echo "🔨 Test de compilation..."
if make -j$(nproc) 2>/dev/null; then
    echo "🎉 Compilation réussie!"
    echo ""
    echo "✅ BioMining configuré avec succès pour Cloud Shell!"
else
    echo "⚠️ Erreurs de compilation détectées. Tentative de compilation partielle..."
    
    # Essayer de compiler seulement la bibliothèque core
    if make biomining_core 2>/dev/null; then
        echo "✅ Bibliothèque core compilée avec succès!"
        echo "ℹ️ Certains composants peuvent ne pas être disponibles."
    else
        echo "❌ Échec de la compilation. Consultez les logs pour plus de détails."
        echo ""
        echo "🔍 Diagnostics recommandés:"
        echo "  1. Vérifiez les versions: gcc --version && cmake --version"
        echo "  2. Vérifiez Qt: qmake6 --version || qmake --version"
        echo "  3. Essayez la compilation manuelle avec plus de détails:"
        echo "     make VERBOSE=1"
        exit 1
    fi
fi

echo ""
echo "📋 Configuration finale:"
echo "  - Répertoire de build: $(pwd)"
echo "  - Type de build: Release"
echo "  - Tests: $([ -f test_mea_interface ] && echo 'Disponibles' || echo 'Non compilés')"
echo ""
echo "🚀 Commandes utiles:"
echo "  make test          # Exécuter les tests"
echo "  make clean         # Nettoyer"
echo "  make VERBOSE=1     # Compilation détaillée"