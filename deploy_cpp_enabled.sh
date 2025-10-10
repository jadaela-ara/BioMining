#!/bin/bash
# ================================================================
# DÉPLOIEMENT AVEC BINDINGS C++ - BIOMINING WEB INTERFACE
# Script de déploiement avec compilation des bindings C++
# ================================================================

set -euo pipefail

echo "🚀 DÉPLOIEMENT BIOMINING AVEC BINDINGS C++"
echo "============================================="

# Configuration
SERVICE_NAME="biomining-cpp"
REGION="europe-west1"

# Fonction pour obtenir le PROJECT_ID
get_project_id() {
    PROJECT_ID=""
    
    # Essayer depuis l'environnement
    if [ -n "${GOOGLE_CLOUD_PROJECT:-}" ]; then
        PROJECT_ID="$GOOGLE_CLOUD_PROJECT"
        echo "✅ Utilisation du PROJECT_ID: $PROJECT_ID (depuis GOOGLE_CLOUD_PROJECT)"
        return
    fi
    
    # Essayer depuis gcloud config
    PROJECT_ID=$(gcloud config get-value project 2>/dev/null || echo "")
    if [ -n "$PROJECT_ID" ]; then
        echo "✅ Utilisation du PROJECT_ID: $PROJECT_ID (depuis gcloud config)"
        return
    fi
    
    # Demander à l'utilisateur
    echo "❌ Aucun projet GCP configuré"
    echo ""
    echo "Vos projets disponibles :"
    gcloud projects list --format="table(projectId,name)" 2>/dev/null || {
        echo "   Erreur: Vous devez vous authentifier avec 'gcloud auth login'"
        exit 1
    }
    echo ""
    read -p "Entrez votre PROJECT_ID : " PROJECT_ID
    
    if [ -z "$PROJECT_ID" ]; then
        echo "❌ PROJECT_ID requis"
        exit 1
    fi
    
    gcloud config set project "$PROJECT_ID"
    echo "✅ Projet configuré: $PROJECT_ID"
}

# Vérifications
echo "🔍 Vérifications..."

# Vérifier gcloud
if ! command -v gcloud &> /dev/null; then
    echo "❌ gcloud CLI manquant. Installez-le depuis: https://cloud.google.com/sdk/docs/install"
    exit 1
fi

# Obtenir le PROJECT_ID
get_project_id

# Vérifier auth
if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | head -1 | grep -q .; then
    echo "❌ Non authentifié. Exécutez: gcloud auth login"
    exit 1
fi

# Vérifier les fichiers C++
required_files=(
    "requirements-cpp.txt"
    "Dockerfile.cpp-enabled" 
    "web/api/server.py"
    "web/index.html"
    "python_bindings/setup.py"
    "python_bindings/biomining_python.cpp"
    "include/bio/biological_network.h"
    "include/crypto/hybrid_bitcoin_miner.h"
    "src/cpp/biological_network.cpp"
    "src/cpp/hybrid_bitcoin_miner.cpp"
)

for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Fichier manquant: $file"
        exit 1
    fi
done

echo "✅ Tous les fichiers C++ présents"
echo ""
echo "📊 Configuration:"
echo "   - Projet: $PROJECT_ID"
echo "   - Service: $SERVICE_NAME"
echo "   - Région: $REGION"
echo "   - Bindings C++: Activés"
echo ""

# Activer les services
echo "🔧 Activation des services Google Cloud..."
gcloud services enable cloudbuild.googleapis.com run.googleapis.com artifactregistry.googleapis.com --quiet

# Sauvegarder et utiliser le Dockerfile C++
echo "📦 Préparation du conteneur avec bindings C++..."

BACKUP_NEEDED=false
if [ -f "Dockerfile" ]; then
    mv Dockerfile Dockerfile.backup
    BACKUP_NEEDED=true
fi

# Copier le Dockerfile C++
cp Dockerfile.cpp-enabled Dockerfile

# Sauvegarder et utiliser les requirements C++
BACKUP_REQ_NEEDED=false
if [ -f "requirements.txt" ]; then
    mv requirements.txt requirements.backup.txt
    BACKUP_REQ_NEEDED=true
fi

# Copier les requirements C++
cp requirements-cpp.txt requirements.txt

echo "🚀 Déploiement avec compilation C++..."
echo "⏳ Cela peut prendre 5-10 minutes (compilation C++)..."

# Déployer avec plus de ressources pour la compilation
gcloud run deploy "$SERVICE_NAME" \
    --source . \
    --platform managed \
    --region "$REGION" \
    --allow-unauthenticated \
    --memory "4Gi" \
    --cpu "4" \
    --port 8080 \
    --timeout=3600 \
    --concurrency=80 \
    --max-instances=10 \
    --quiet \
    2>/dev/null

deployment_success=$?

# Nettoyer
rm -f Dockerfile requirements.txt
if [ "$BACKUP_NEEDED" = true ]; then
    mv Dockerfile.backup Dockerfile
fi
if [ "$BACKUP_REQ_NEEDED" = true ]; then
    mv requirements.backup.txt requirements.txt
fi

# Résultats
if [ $deployment_success -eq 0 ]; then
    echo ""
    echo "🎉 SUCCÈS ! Bindings C++ déployés !"
    echo "=================================="
    
    # Obtenir l'URL
    SERVICE_URL=$(gcloud run services describe "$SERVICE_NAME" \
        --region="$REGION" \
        --format="value(status.url)" 2>/dev/null)
    
    echo ""
    echo "🌐 URL de votre application: $SERVICE_URL"
    echo "📊 API Status: $SERVICE_URL/api/status"
    echo "🔧 Bindings C++: ACTIVÉS (biomining_cpp module)"
    echo ""
    echo "✅ Votre plateforme BioMining avec vrais bindings C++ est en ligne !"
    
    # Test des bindings
    if command -v curl &> /dev/null; then
        echo ""
        echo "🧪 Test des bindings C++..."
        sleep 10
        
        # Test API status
        if response=$(curl -f -s "$SERVICE_URL/api/status" 2>/dev/null); then
            echo "✅ API fonctionnelle !"
            
            # Vérifier si les bindings C++ sont chargés
            if echo "$response" | grep -q '"bindings.*true"' 2>/dev/null; then
                echo "✅ Bindings C++ chargés avec succès !"
            else
                echo "⚠️  Bindings C++ en cours de chargement..."
            fi
        else
            echo "⚠️  Application en cours de démarrage..."
        fi
        
        echo ""
        echo "🎯 Pour tester l'apprentissage BiologicalNetwork:"
        echo "   1. Allez sur: $SERVICE_URL"
        echo "   2. Démarrez le système biological"  
        echo "   3. Lancez un training - il utilisera maintenant les VRAIS bindings C++ !"
    fi
    
else
    echo ""
    echo "❌ ÉCHEC DU DÉPLOIEMENT"
    echo "====================="
    echo ""
    echo "💡 Solutions :"
    echo "   1. Vérifiez que la facturation est activée"
    echo "   2. Augmentez les quotas Cloud Build"
    echo "   3. Essayez une région différente"
    echo "   4. Vérifiez les logs : gcloud run logs read --service=$SERVICE_NAME --region=$REGION"
    exit 1
fi