#!/bin/bash

echo "🧪 Test de configuration CMake pour Cloud Shell..."

# Nettoyer
rm -rf build_test/
mkdir -p build_test && cd build_test

echo "📋 Test 1: Configuration standard..."
if cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON 2>/dev/null; then
    echo "✅ Configuration standard réussie!"
    STANDARD_OK=true
else
    echo "❌ Configuration standard échouée"
    STANDARD_OK=false
fi

# Nettoyer pour le test suivant
rm -rf *

echo "📋 Test 2: Configuration sans Boost..."
if cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_Boost=TRUE 2>/dev/null; then
    echo "✅ Configuration sans Boost réussie!"
    NO_BOOST_OK=true
else
    echo "❌ Configuration sans Boost échouée"
    NO_BOOST_OK=false
fi

# Nettoyer pour le test suivant
rm -rf *

echo "📋 Test 3: Configuration minimale..."
if cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=OFF \
    -DCMAKE_DISABLE_FIND_PACKAGE_Boost=TRUE \
    -DCMAKE_DISABLE_FIND_PACKAGE_XKB=TRUE 2>/dev/null; then
    echo "✅ Configuration minimale réussie!"
    MINIMAL_OK=true
else
    echo "❌ Configuration minimale échouée"
    MINIMAL_OK=false
fi

cd ..
rm -rf build_test/

echo ""
echo "📊 Résultats des tests:"
echo "  Configuration standard: $([ "$STANDARD_OK" = true ] && echo '✅ OK' || echo '❌ Échec')"
echo "  Configuration sans Boost: $([ "$NO_BOOST_OK" = true ] && echo '✅ OK' || echo '❌ Échec')"
echo "  Configuration minimale: $([ "$MINIMAL_OK" = true ] && echo '✅ OK' || echo '❌ Échec')"

if [ "$STANDARD_OK" = true ]; then
    echo ""
    echo "🎉 Configuration standard recommandée:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON"
    echo "  make -j\$(nproc)"
elif [ "$NO_BOOST_OK" = true ]; then
    echo ""
    echo "⚠️ Configuration sans Boost recommandée:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DCMAKE_DISABLE_FIND_PACKAGE_Boost=TRUE"
    echo "  make -j\$(nproc)"
elif [ "$MINIMAL_OK" = true ]; then
    echo ""
    echo "🔧 Configuration minimale recommandée:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF -DCMAKE_DISABLE_FIND_PACKAGE_Boost=TRUE"
    echo "  make -j\$(nproc)"
else
    echo ""
    echo "❌ Aucune configuration ne fonctionne. Vérifiez les dépendances:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y build-essential cmake qt6-base-dev libssl-dev"
fi