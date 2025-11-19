#!/bin/bash

# Script de déploiement SANS psutil - Solution CORRIGÉE
echo "🚀 Déploiement CloudRun SANS psutil - SOLUTION CORRIGÉE"

set -euo pipefail

# Configuration - Variables modifiables
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-}"
SERVICE_NAME="biomining-web"
REGION="europe-west1"
DOCKERFILE="Dockerfile.no-psutil"

# Fonction pour obtenir le PROJECT_ID automatiquement
get_project_id() {
    if [ -z "$PROJECT_ID" ]; then
        # Essayer de récupérer le projet actuel
        PROJECT_ID=$(gcloud config get-value project 2>/dev/null || echo "")
        
        if [ -z "$PROJECT_ID" ]; then
            echo "❌ Aucun projet GCP configuré."
            echo "💡 Exécutez une de ces commandes :"
            echo "   export GOOGLE_CLOUD_PROJECT=votre-projet-id"
            echo "   gcloud config set project votre-projet-id"
            echo "   gcloud projects list  # pour voir vos projets"
            exit 1
        fi
    fi
    echo "✅ Utilisation du projet: $PROJECT_ID"
}

# Vérifications préliminaires
echo "🔍 Vérifications préliminaires..."

# Vérifier gcloud CLI
if ! command -v gcloud &> /dev/null; then
    echo "❌ gcloud CLI non trouvé. Installez-le depuis:"
    echo "   https://cloud.google.com/sdk/docs/install"
    exit 1
fi

# Récupérer le PROJECT_ID
get_project_id

# Vérifier l'authentification
if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | grep -q .; then
    echo "❌ Vous n'êtes pas authentifié avec gcloud."
    echo "💡 Exécutez: gcloud auth login"
    exit 1
fi

# Configurer le projet
echo "📋 Configuration du projet GCP..."
gcloud config set project $PROJECT_ID

# Activer les services nécessaires
echo "🔧 Activation des services Google Cloud..."
gcloud services enable cloudbuild.googleapis.com
gcloud services enable run.googleapis.com
gcloud services enable containerregistry.googleapis.com

# Configurer Docker auth
echo "🔐 Configuration de l'authentification Docker..."
gcloud auth configure-docker --quiet

# Vérifier que le Dockerfile existe
if [ ! -f "$DOCKERFILE" ]; then
    echo "❌ Dockerfile non trouvé: $DOCKERFILE"
    exit 1
fi

echo "✅ Toutes les vérifications passées"
echo "📊 Configuration:"
echo "   - Projet: $PROJECT_ID"
echo "   - Service: $SERVICE_NAME"
echo "   - Région: $REGION"
echo "   - Dockerfile: $DOCKERFILE"
echo ""

# Construire et pousser l'image
echo "🔧 Construction de l'image Docker..."

# Sauvegarder le Dockerfile existant s'il y en a un
if [ -f "Dockerfile" ]; then
    mv Dockerfile Dockerfile.backup
    BACKUP_NEEDED=true
else
    BACKUP_NEEDED=false
fi

# Copier le Dockerfile spécifique comme Dockerfile principal
cp $DOCKERFILE Dockerfile

gcloud builds submit \
    --tag gcr.io/$PROJECT_ID/$SERVICE_NAME \
    --timeout=600s \
    .

# Restaurer le Dockerfile original
rm -f Dockerfile
if [ "$BACKUP_NEEDED" = true ]; then
    mv Dockerfile.backup Dockerfile
fi

if [ $? -ne 0 ]; then
    echo "❌ Échec de la construction de l'image"
    exit 1
fi

echo "✅ Image construite avec succès"

# Déployer sur Cloud Run
echo "🚀 Déploiement sur Cloud Run..."
gcloud run deploy $SERVICE_NAME \
    --image gcr.io/$PROJECT_ID/$SERVICE_NAME \
    --platform managed \
    --region $REGION \
    --allow-unauthenticated \
    --memory 2Gi \
    --cpu 2 \
    --port 8080 \
    --timeout 600s \
    --concurrency 50 \
    --max-instances 5 \
    --set-env-vars="PORT=8080,NODE_ENV=production,BIOMINING_ENVIRONMENT=production" \
    --labels="app=biomining-web,version=1-0-0"

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Déploiement réussi !"
    echo "🎉 Application déployée avec succès !"
    echo ""
    
    # Récupérer l'URL du service
    SERVICE_URL=$(gcloud run services describe $SERVICE_NAME --region=$REGION --format="value(status.url)")
    
    echo "🌐 URLs de votre application:"
    echo "   🏠 Interface principale: $SERVICE_URL/"
    echo "   📊 API Status:          $SERVICE_URL/api/status"
    echo "   🔌 WebSocket:           wss://${SERVICE_URL#https://}/ws/hybrid-mining"
    echo ""
    
    # Test de santé
    echo "🧪 Test de l'application..."
    if curl -f -s "$SERVICE_URL/api/status" > /dev/null; then
        echo "✅ Application en ligne et fonctionnelle!"
    else
        echo "⚠️  L'application démarre encore... Attendez 30 secondes et testez l'URL"
    fi
    
else
    echo "❌ Échec du déploiement"
    echo "💡 Vérifiez les logs avec:"
    echo "   gcloud logging read 'resource.type=\"cloud_run_revision\"' --limit 50"
    exit 1
fi