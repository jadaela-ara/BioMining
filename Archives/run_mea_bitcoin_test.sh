#!/bin/bash
# Script de lancement pour les tests MEA Bitcoin Learning
# BioMining Platform - Test d'apprentissage réseau biologique pour mining Bitcoin

set -e

echo "🧬 === BIOMINING MEA BITCOIN LEARNING TEST ==="
echo "🔗 Test d'apprentissage réseau biologique pour mining Bitcoin"
echo "📚 Utilisation de blocs et nonces Bitcoin réels"
echo "=================================================="

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_SCRIPT="$SCRIPT_DIR/tests/test_mea_bitcoin_learning.py"
CONFIG_FILE="$SCRIPT_DIR/config/mea_bitcoin_test.json"
RESULTS_DIR="$SCRIPT_DIR/tests/results"

# Vérifications préliminaires
echo "🔍 Vérifications préliminaires..."

if [ ! -f "$TEST_SCRIPT" ]; then
    echo "❌ Script de test non trouvé: $TEST_SCRIPT"
    exit 1
fi

if [ ! -f "$CONFIG_FILE" ]; then
    echo "❌ Fichier de configuration non trouvé: $CONFIG_FILE"
    exit 1
fi

# Créer le répertoire de résultats si nécessaire
mkdir -p "$RESULTS_DIR"

echo "✅ Script de test: $TEST_SCRIPT"
echo "✅ Configuration: $CONFIG_FILE"
echo "✅ Répertoire résultats: $RESULTS_DIR"

# Vérifier les dépendances Python
echo ""
echo "📦 Vérification des dépendances Python..."

python3 -c "import numpy; print(f'✅ NumPy {numpy.__version__}')" || {
    echo "❌ NumPy non installé. Installation..."
    pip3 install numpy
}

python3 -c "import hashlib; print('✅ hashlib disponible')" || {
    echo "❌ hashlib non disponible"
    exit 1
}

# Options du script
QUICK_TEST=${1:-false}
DIFFICULTY_LEVELS=${2:-"0,1,2"}
MAX_ITERATIONS=${3:-1000}

echo ""
echo "🎯 Paramètres de test:"
echo "   Test rapide: $QUICK_TEST"
echo "   Niveaux de difficulté: $DIFFICULTY_LEVELS"
echo "   Itérations max: $MAX_ITERATIONS"

# Export des variables d'environnement
export BIOMINING_CONFIG_PATH="$CONFIG_FILE"
export BIOMINING_RESULTS_DIR="$RESULTS_DIR"
export PYTHONPATH="$SCRIPT_DIR/src:$PYTHONPATH"

# Fonction de nettoyage
cleanup() {
    echo ""
    echo "🧹 Nettoyage..."
    # Tuer les processus MEA si nécessaire
    pkill -f "mea_bitcoin_learning" 2>/dev/null || true
    echo "✅ Nettoyage terminé"
}

# Configurer le piège pour le nettoyage
trap cleanup EXIT INT TERM

# Lancement du test
echo ""
echo "🚀 Lancement du test MEA Bitcoin Learning..."
echo "📊 Monitoring en temps réel disponible dans le log"
echo "⏱️  Début: $(date)"
echo ""

if [ "$QUICK_TEST" = "true" ]; then
    echo "⚡ Mode test rapide activé"
    python3 "$TEST_SCRIPT" --quick --max-iterations 50
else
    echo "🎓 Mode entraînement complet"
    python3 "$TEST_SCRIPT" --difficulty-levels "$DIFFICULTY_LEVELS" --max-iterations "$MAX_ITERATIONS"
fi

TEST_EXIT_CODE=$?

echo ""
echo "⏱️  Fin: $(date)"

# Résumé des résultats
if [ $TEST_EXIT_CODE -eq 0 ]; then
    echo "🎉 === TEST TERMINÉ AVEC SUCCÈS ==="
    
    # Afficher les derniers résultats
    LATEST_RESULT=$(find "$RESULTS_DIR" -name "*.json" -type f -printf '%T@ %p\n' 2>/dev/null | sort -n | tail -1 | cut -d' ' -f2-)
    
    if [ -n "$LATEST_RESULT" ] && [ -f "$LATEST_RESULT" ]; then
        echo ""
        echo "📁 Dernier fichier de résultats:"
        echo "   $LATEST_RESULT"
        echo ""
        echo "📊 Résumé rapide:"
        
        # Extraire quelques métriques clés du JSON
        python3 -c "
import json
try:
    with open('$LATEST_RESULT', 'r') as f:
        data = json.load(f)
    
    if 'final_stats' in data:
        stats = data['final_stats']
        print(f'   Blocs traités: {stats.get(\"blocks_processed\", 0)}')
        print(f'   Nonces valides: {stats.get(\"valid_nonces_found\", 0)}')
        print(f'   Taux de succès: {stats.get(\"success_rate\", 0):.2%}')
        print(f'   Tentatives totales: {stats.get(\"total_attempts\", 0)}')
    
    if 'level_results' in data:
        levels = len(data['level_results'])
        print(f'   Niveaux testés: {levels}')
        
        successes = sum(1 for level in data['level_results'] if level.get('success', False))
        print(f'   Niveaux réussis: {successes}/{levels}')
        
except Exception as e:
    print(f'   Erreur lecture résultats: {e}')
" 2>/dev/null || echo "   (Impossible de lire le résumé automatiquement)"
        
    else
        echo "⚠️  Aucun fichier de résultats trouvé"
    fi
    
    echo ""
    echo "📚 Consultez les logs détaillés:"
    echo "   tail -f tests/mea_bitcoin_learning.log"
    
else
    echo "❌ === TEST ÉCHOUÉ ==="
    echo "📋 Code de sortie: $TEST_EXIT_CODE"
    echo "📚 Consultez les logs pour plus de détails:"
    echo "   tail -n 50 tests/mea_bitcoin_learning.log"
fi

echo ""
echo "📂 Fichiers générés:"
ls -la "$RESULTS_DIR" 2>/dev/null || echo "   Aucun fichier de résultat généré"

echo ""
echo "🔚 Script terminé"
exit $TEST_EXIT_CODE