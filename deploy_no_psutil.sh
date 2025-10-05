#!/bin/bash

# Script de déploiement SANS psutil - Solution GARANTIE
echo "🚀 Déployment CloudRun SANS psutil - SOLUTION GARANTIE"

# Configuration
PROJECT_ID="biological-bitcoin-mining"
SERVICE_NAME="biomining-web"
REGION="us-central1"
DOCKERFILE="Dockerfile.no-psutil"

# Vérifier que le projet GCP est configuré
echo "📋 Configuration du projet GCP..."
gcloud config set project $PROJECT_ID

# Construire et déployer avec Docker SANS psutil
echo "🔧 Construction de l'image Docker SANS psutil..."
gcloud builds submit --tag gcr.io/$PROJECT_ID/$SERVICE_NAME --dockerfile=$DOCKERFILE .

if [ $? -ne 0 ]; then
    echo "❌ Échec de la construction de l'image"
    exit 1
fi

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
    --set-env-vars="PORT=8080,ENVIRONMENT=production"

if [ $? -eq 0 ]; then
    echo "✅ Déploiement réussi !"
    echo "🌐 Service URL:"
    gcloud run services describe $SERVICE_NAME --region=$REGION --format="value(status.url)"
else
    echo "❌ Échec du déploiement"
    exit 1
fi