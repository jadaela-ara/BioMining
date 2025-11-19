#!/bin/bash
# ================================================================
# DÉPLOIEMENT ULTRA-SIMPLE - BIOMINING WEB INTERFACE
# Script de déploiement minimal avec le moins de complications
# ================================================================

set -euo pipefail

echo "🚀 DÉPLOIEMENT ULTRA-SIMPLE - BIOMINING WEB INTERFACE"
echo "===================================================="

# Configuration
SERVICE_NAME="biomining-web"
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

# Vérifications rapides
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

# Vérifier les fichiers essentiels
required_files=("requirements-no-psutil.txt" "web/api/server.py" "web/index.html")
for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Fichier manquant: $file"
        exit 1
    fi
done

echo "✅ Vérifications terminées"
echo ""
echo "📊 Configuration:"
echo "   - Projet: $PROJECT_ID"
echo "   - Service: $SERVICE_NAME"
echo "   - Région: $REGION"
echo ""

# Activer les services (silencieux)
echo "🔧 Activation des services Google Cloud..."
gcloud services enable cloudbuild.googleapis.com run.googleapis.com --quiet

# Utiliser le Dockerfile.no-psutil existant renommé
echo "📦 Préparation du conteneur..."

# Sauvegarder le Dockerfile existant s'il y en a un
BACKUP_NEEDED=false
if [ -f "Dockerfile" ]; then
    mv Dockerfile Dockerfile.backup
    BACKUP_NEEDED=true
fi

# Copier notre Dockerfile optimisé
cp Dockerfile.no-psutil Dockerfile

echo "🚀 Déploiement en cours..."
echo "⏳ Cela peut prendre 2-5 minutes..."

# Déployer
gcloud run deploy "$SERVICE_NAME" \
    --source . \
    --platform managed \
    --region "$REGION" \
    --allow-unauthenticated \
    --memory "2Gi" \
    --cpu "2" \
    --port 8080 \
    --quiet \
    2>/dev/null

deployment_success=$?

# Nettoyer
rm -f Dockerfile
if [ "$BACKUP_NEEDED" = true ]; then
    mv Dockerfile.backup Dockerfile
fi

# Résultats
if [ $deployment_success -eq 0 ]; then
    echo ""
    echo "🎉 SUCCÈS ! Votre application est déployée !"
    echo "==========================================="
    
    # Obtenir l'URL
    SERVICE_URL=$(gcloud run services describe "$SERVICE_NAME" \
        --region="$REGION" \
        --format="value(status.url)" 2>/dev/null)
    
    echo ""
    echo "🌐 URL de votre application: $SERVICE_URL"
    echo "📊 API Status: $SERVICE_URL/api/status"
    echo ""
    echo "✅ Votre interface de minage Bitcoin biologique est en ligne !"
    
    # Test rapide
    if command -v curl &> /dev/null; then
        echo ""
        echo "🧪 Test rapide..."
        sleep 5
        if curl -f -s "$SERVICE_URL/api/status" > /dev/null 2>&1; then
            echo "✅ Application fonctionnelle !"
        else
            echo "⚠️  Application en cours de démarrage (normal)"
        fi
    fi
    
else
    echo ""
    echo "❌ ÉCHEC DU DÉPLOIEMENT"
    echo "====================="
    echo ""
    echo "💡 Solutions rapides :"
    echo "   1. Vérifiez que la facturation est activée sur votre projet"
    echo "   2. Essayez une autre région :"
    echo "      GOOGLE_CLOUD_REGION=us-central1 $0"
    echo "   3. Vérifiez les quotas dans la console GCP"
    exit 1
fi