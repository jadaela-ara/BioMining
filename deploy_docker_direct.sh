#!/bin/bash

# Script de déploiement Docker direct SANS psutil
echo "🚀 Construction Docker SANS psutil - TEST LOCAL"

# Configuration
IMAGE_NAME="biomining-web-no-psutil"
DOCKERFILE="Dockerfile.no-psutil"

# Vérifier que Docker fonctionne
if ! docker --version > /dev/null 2>&1; then
    echo "❌ Docker n'est pas disponible"
    exit 1
fi

echo "🔧 Construction de l'image Docker SANS psutil..."
docker build -f $DOCKERFILE -t $IMAGE_NAME .

if [ $? -ne 0 ]; then
    echo "❌ Échec de la construction de l'image Docker"
    exit 1
fi

echo "✅ Image Docker construite avec succès !"
echo "📋 Informations sur l'image :"
docker images | grep $IMAGE_NAME

echo ""
echo "🧪 Pour tester localement :"
echo "docker run -p 8080:8080 $IMAGE_NAME"
echo ""
echo "🌐 Pour déployer sur CloudRun (nécessite gcloud) :"
echo "1. gcloud builds submit --tag gcr.io/PROJECT_ID/biomining-web ."
echo "2. gcloud run deploy biomining-web --image gcr.io/PROJECT_ID/biomining-web --region us-central1"