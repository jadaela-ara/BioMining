#!/bin/bash

# 🎯 Script de compilation sélective des exemples BioMining
# Compile uniquement les exemples qui sont corrigés et fonctionnels

set -e

echo "🎯 BioMining - Compilation Sélective des Exemples"
echo "================================================="
echo ""

# Vérifier si nous sommes dans le bon répertoire
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Erreur: Exécutez ce script depuis le répertoire racine du projet BioMining"
    exit 1
fi

# Aller dans le répertoire build
if [ ! -d "build" ]; then
    echo "📁 Création du répertoire build..."
    mkdir build
fi

cd build

# Configuration CMake avec exemples activés
echo "⚙️  Configuration CMake avec exemples..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo ""
echo "🔨 Compilation des exemples fonctionnels..."

# Liste des exemples qui compilent correctement
WORKING_EXAMPLES=(
    "example_mea_basic"
    "example_mining_basic" 
    "example_complete_workflow"
)

# Liste des exemples avec problèmes connus
BROKEN_EXAMPLES=(
    "example_custom_benchmark"
)

echo "✅ Exemples à compiler:"
for example in "${WORKING_EXAMPLES[@]}"; do
    echo "  - $example"
done

echo ""
echo "⚠️  Exemples ignorés (nécessitent corrections):"
for example in "${BROKEN_EXAMPLES[@]}"; do
    echo "  - $example"
done

echo ""

# Compilation des exemples fonctionnels
SUCCESS_COUNT=0
TOTAL_COUNT=${#WORKING_EXAMPLES[@]}

for example in "${WORKING_EXAMPLES[@]}"; do
    echo "🔨 Compilation de $example..."
    if make "$example" -j1; then
        echo "  ✅ $example compilé avec succès"
        ((SUCCESS_COUNT++))
    else
        echo "  ❌ Échec de compilation de $example"
    fi
    echo ""
done

echo "📊 Résumé de Compilation"
echo "======================="
echo "Exemples réussis : $SUCCESS_COUNT/$TOTAL_COUNT"

if [ -f "bin/example_mea_basic" ]; then
    echo "  ✅ example_mea_basic ($(ls -lh bin/example_mea_basic | awk '{print $5}'))"
fi

if [ -f "bin/example_mining_basic" ]; then
    echo "  ✅ example_mining_basic ($(ls -lh bin/example_mining_basic | awk '{print $5}'))"
fi

if [ -f "bin/example_complete_workflow" ]; then
    echo "  ✅ example_complete_workflow ($(ls -lh bin/example_complete_workflow | awk '{print $5}'))"
fi

echo ""
echo "🚀 Pour tester les exemples compilés:"
echo "  ./bin/example_mea_basic"
echo "  ./bin/example_mining_basic"  
echo "  ./bin/example_complete_workflow"
echo ""

if [ $SUCCESS_COUNT -eq $TOTAL_COUNT ]; then
    echo "🎉 Tous les exemples fonctionnels ont été compilés avec succès!"
    exit 0
else
    echo "⚠️  $((TOTAL_COUNT - SUCCESS_COUNT)) exemple(s) ont échoué"
    exit 1
fi