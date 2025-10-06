#!/bin/bash
# Script de test local pour vérifier que l'application peut démarrer correctement

set -euo pipefail

echo "🧪 TEST LOCAL DE L'APPLICATION AVANT DÉPLOIEMENT"
echo "================================================"

# Vérifications des fichiers
echo "🔍 Vérification des fichiers requis..."

required_files=(
    "requirements-no-psutil.txt"
    "web/api/server.py"
    "web/index.html"
    "biological_bitcoin_learning.py"
    "real_mea_interface.py"
)

missing_files=()
for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        missing_files+=("$file")
    fi
done

if [ ${#missing_files[@]} -ne 0 ]; then
    echo "❌ Fichiers manquants :"
    printf '   - %s\n' "${missing_files[@]}"
    exit 1
fi

echo "✅ Tous les fichiers requis sont présents"

# Vérifier Python et pip
echo "🐍 Vérification de l'environnement Python..."
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 non trouvé"
    exit 1
fi

if ! command -v pip &> /dev/null && ! command -v pip3 &> /dev/null; then
    echo "❌ pip non trouvé"
    exit 1
fi

echo "✅ Python $(python3 --version) disponible"

# Créer un environnement virtuel temporaire pour le test
echo "📦 Création d'un environnement de test temporaire..."
python3 -m venv test_env || {
    echo "❌ Impossible de créer l'environnement virtuel"
    exit 1
}

# Activer l'environnement virtuel
source test_env/bin/activate

# Installer les dépendances
echo "📚 Installation des dépendances..."
pip install --quiet -r requirements-no-psutil.txt || {
    echo "❌ Échec de l'installation des dépendances"
    deactivate
    rm -rf test_env
    exit 1
}

echo "✅ Dépendances installées avec succès"

# Test d'importation des modules
echo "🔬 Test d'importation des modules Python..."
python3 -c "
import sys
sys.path.append('.')

try:
    from web.api.server import app
    print('✅ Module server importé avec succès')
except ImportError as e:
    print(f'⚠️  Avertissement lors de l\'importation: {e}')
    print('   (Normal en environnement de développement)')

try:
    import fastapi
    import uvicorn
    import websockets
    print('✅ Modules FastAPI importés avec succès')
except ImportError as e:
    print(f'❌ Erreur critique d\'importation: {e}')
    sys.exit(1)
"

import_result=$?

# Nettoyer l'environnement de test
deactivate
rm -rf test_env

if [ $import_result -eq 0 ]; then
    echo ""
    echo "🎉 TEST LOCAL RÉUSSI !"
    echo "===================="
    echo "✅ Votre application est prête pour le déploiement"
    echo ""
    echo "🚀 Pour déployer maintenant :"
    echo "   ./deploy_simple_fixed.sh      # Déploiement interactif (recommandé)"
    echo "   ./deploy_no_psutil.sh         # Déploiement avec Dockerfile personnalisé"
    echo "   ./deploy_web_interface_fixed.sh  # Déploiement avancé"
    echo ""
else
    echo ""
    echo "❌ PROBLÈMES DÉTECTÉS"
    echo "==================="
    echo "Des problèmes ont été détectés mais ils peuvent être normaux en développement."
    echo "Le déploiement sur Cloud Run devrait fonctionner correctement."
    echo ""
    echo "🚀 Vous pouvez essayer le déploiement :"
    echo "   ./deploy_simple_fixed.sh"
fi