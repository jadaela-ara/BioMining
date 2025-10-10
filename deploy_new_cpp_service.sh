#!/bin/bash
# ================================================================
# DÉPLOIEMENT NOUVEAU SERVICE AVEC BINDINGS C++
# Script pour créer un nouveau service Cloud Run avec C++ bindings
# ================================================================

set -euo pipefail

echo "🚀 CRÉATION NOUVEAU SERVICE BIOMINING C++"
echo "=========================================="

# Configuration
SERVICE_NAME="biomining-cpp-bindings"
REGION="europe-west1"
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-$(gcloud config get-value project)}"

if [ -z "$PROJECT_ID" ]; then
    echo "❌ PROJECT_ID non configuré"
    echo "Exécutez: gcloud config set project VOTRE-PROJECT-ID"
    exit 1
fi

echo "📊 Configuration:"
echo "   - Projet: $PROJECT_ID"
echo "   - Service: $SERVICE_NAME"
echo "   - Région: $REGION"
echo "   - Dockerfile: Dockerfile.cpp-enabled"
echo ""

# Vérifier les fichiers C++
echo "🔍 Vérification des fichiers C++..."
required_files=(
    "Dockerfile.cpp-enabled"
    "requirements-cpp.txt" 
    "python_bindings/biomining_python.cpp"
    "src/cpp/biological_network.cpp"
    "include/bio/biological_network.h"
)

for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Fichier manquant: $file"
        exit 1
    fi
done
echo "✅ Tous les fichiers C++ présents"

# Activer les services
echo "🔧 Activation des services..."
gcloud services enable cloudbuild.googleapis.com run.googleapis.com --quiet

# Préparer les fichiers pour le build
echo "📦 Préparation du build C++..."

# Sauvegarder les fichiers existants
BACKUP_DOCKERFILE=""
BACKUP_REQUIREMENTS=""

if [ -f "Dockerfile" ]; then
    cp Dockerfile Dockerfile.backup.original
    BACKUP_DOCKERFILE="Dockerfile.backup.original"
fi

if [ -f "requirements.txt" ]; then
    cp requirements.txt requirements.backup.original.txt
    BACKUP_REQUIREMENTS="requirements.backup.original.txt"
fi

# Utiliser les fichiers C++
cp Dockerfile.cpp-enabled Dockerfile
cp requirements-cpp.txt requirements.txt

echo "🚀 Déploiement du nouveau service avec C++ bindings..."
echo "⏳ Compilation C++ en cours (5-10 minutes)..."

# Créer le cloudbuild.yaml pour un build plus robuste
cat > cloudbuild.yaml << EOF
steps:
- name: 'gcr.io/cloud-builders/docker'
  args: ['build', '-t', 'gcr.io/$PROJECT_ID/$SERVICE_NAME', '-f', 'Dockerfile', '.']
  timeout: '3600s'
- name: 'gcr.io/cloud-builders/docker'
  args: ['push', 'gcr.io/$PROJECT_ID/$SERVICE_NAME']

options:
  machineType: 'E2_HIGHCPU_8'
  diskSizeGb: '100'
  
timeout: '3600s'
EOF

# Build avec Cloud Build
gcloud builds submit --config cloudbuild.yaml --timeout=3600s

# Déployer sur Cloud Run
gcloud run deploy "$SERVICE_NAME" \
    --image "gcr.io/$PROJECT_ID/$SERVICE_NAME" \
    --platform managed \
    --region "$REGION" \
    --allow-unauthenticated \
    --memory "4Gi" \
    --cpu "4" \
    --port 8080 \
    --timeout=3600 \
    --concurrency=80 \
    --max-instances=10 \
    --set-env-vars="QT_QPA_PLATFORM=offscreen,BIOMINING_ENVIRONMENT=production,PYTHONPATH=/app" \
    --quiet

deployment_success=$?

# Nettoyer
rm -f Dockerfile requirements.txt cloudbuild.yaml
if [ -n "$BACKUP_DOCKERFILE" ]; then
    mv "$BACKUP_DOCKERFILE" Dockerfile
fi
if [ -n "$BACKUP_REQUIREMENTS" ]; then
    mv "$BACKUP_REQUIREMENTS" requirements.txt
fi

# Résultats
if [ $deployment_success -eq 0 ]; then
    echo ""
    echo "🎉 SUCCÈS ! SERVICE C++ DÉPLOYÉ !"
    echo "================================"
    
    # Obtenir l'URL
    SERVICE_URL=$(gcloud run services describe "$SERVICE_NAME" \
        --region="$REGION" \
        --format="value(status.url)")
    
    echo ""
    echo "🌐 Nouveau service C++: $SERVICE_URL"
    echo "📊 API Status: $SERVICE_URL/api/status"
    echo "🔧 Bindings Check: $SERVICE_URL/api/bindings" 
    echo ""
    echo "🧪 Test des bindings C++:"
    echo "curl $SERVICE_URL/api/bindings"
    echo ""
    
    # Test automatique
    echo "🔍 Test automatique des bindings..."
    sleep 30
    
    if response=$(curl -f -s "$SERVICE_URL/api/bindings" 2>/dev/null); then
        if echo "$response" | grep -q '"cpp_available.*true"'; then
            echo "🎊 SUCCÈS ! Bindings C++ chargés !"
            echo "$response" | jq '.'
        else
            echo "⚠️ Bindings encore en cours de chargement..."
            echo "$response" | jq '.'
        fi
    else
        echo "⚠️ Service en cours de démarrage..."
    fi
    
else
    echo ""
    echo "❌ ÉCHEC DU DÉPLOIEMENT"
    echo "====================="
    exit 1
fi