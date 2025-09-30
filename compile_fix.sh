#!/bin/bash

# 🔧 Script de correction de compilation BioMining
# Résout le problème "No rule to make target 'biomining_core'"

set -e

echo "🔧 BioMining - Correction de Compilation"
echo "======================================"
echo ""

# Nettoyage du build précédent
if [ -d "build" ]; then
    echo "🧹 Nettoyage du répertoire build existant..."
    rm -rf build
fi

# Création du répertoire build
echo "📁 Création du répertoire build..."
mkdir -p build
cd build

# Configuration CMake
echo "⚙️  Configuration CMake..."
echo "  - Type de build: Release"
echo "  - Tests activés: ON"
echo "  - Exemples activés: ON"
echo ""

if cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; then
    echo "✅ Configuration CMake réussie!"
else
    echo "❌ Erreur de configuration CMake"
    exit 1
fi

echo ""
echo "🔨 Compilation du projet..."
echo "  - Utilisation de tous les processeurs disponibles: $(nproc)"
echo ""

# Compilation
if make -j$(nproc); then
    echo ""
    echo "🎉 Compilation réussie!"
    echo ""
    echo "✅ Binaires générés :"
    if [ -f "bin/biomining_gui" ]; then
        echo "  ✅ biomining_gui"
    else
        echo "  ❌ biomining_gui"
    fi
    
    if [ -f "bin/biomining_cli" ]; then
        echo "  ✅ biomining_cli"
    else
        echo "  ❌ biomining_cli"
    fi
    
    if [ -f "lib/libbiomining_core.a" ]; then
        echo "  ✅ libbiomining_core.a"
    else
        echo "  ❌ libbiomining_core.a"
    fi
    
    echo ""
    echo "📋 Tests disponibles :"
    for test in test_*; do
        if [ -x "$test" ]; then
            echo "  ✅ $test"
        fi
    done
    
    echo ""
    echo "🚀 Pour lancer les applications :"
    echo "  Interface graphique : ./bin/biomining_gui"
    echo "  Interface CLI       : ./bin/biomining_cli --help"
    echo ""
    echo "🧪 Pour lancer les tests :"
    echo "  Tous les tests      : make test"
    echo "  Test spécifique     : ./test_<nom_du_test>"
    
else
    echo ""
    echo "❌ Erreur de compilation"
    echo ""
    echo "🔍 Conseils de dépannage :"
    echo "  1. Vérifiez que toutes les dépendances sont installées :"
    echo "     ./install_deps_cloudshell.sh"
    echo ""
    echo "  2. Vérifiez les messages d'erreur ci-dessus"
    echo ""
    echo "  3. En cas de problème avec Boost, utilisez :"
    echo "     cmake .. -DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON"
    echo ""
    echo "  4. Pour plus d'aide, consultez CLOUDSHELL_SETUP.md"
    exit 1
fi