#!/bin/bash
# ================================================================
# QUICK DEPLOY - WEB INTERFACE TO CLOUDRUN (VERSION CORRIGÉE)
# Deploy directly from current directory with web interface
# ================================================================

set -euo pipefail

echo "🚀 DEPLOYING HYBRID BITCOIN MINING WEB INTERFACE TO CLOUDRUN"
echo "============================================================="

# Configuration avec valeurs par défaut
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-}"
REGION="${GOOGLE_CLOUD_REGION:-europe-west1}"
SERVICE_NAME="biomining-web"

# Fonction pour obtenir le PROJECT_ID
get_project_id() {
    if [ -z "$PROJECT_ID" ]; then
        # Essayer de récupérer le projet actuel
        PROJECT_ID=$(gcloud config get-value project 2>/dev/null || echo "")
        
        if [ -z "$PROJECT_ID" ]; then
            echo "❌ Aucun projet GCP configuré."
            echo "💡 Options pour configurer le projet :"
            echo "   1. export GOOGLE_CLOUD_PROJECT=votre-projet-id"
            echo "   2. gcloud config set project votre-projet-id"
            echo ""
            echo "📋 Vos projets disponibles :"
            gcloud projects list --format="table(projectId,name)" 2>/dev/null || echo "   Exécutez 'gcloud auth login' si nécessaire"
            exit 1
        fi
    fi
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

echo "✅ Project ID: ${PROJECT_ID}"
echo "✅ Region: ${REGION}"
echo "✅ Service: ${SERVICE_NAME}"
echo ""

# Configurer le projet
gcloud config set project "${PROJECT_ID}"

# Activer les services requis
echo "🔧 Activation des services Google Cloud..."
gcloud services enable cloudbuild.googleapis.com --project="${PROJECT_ID}"
gcloud services enable run.googleapis.com --project="${PROJECT_ID}"
gcloud services enable containerregistry.googleapis.com --project="${PROJECT_ID}"

# Configurer l'authentification Docker
echo "🔐 Configuration de l'authentification Docker..."
gcloud auth configure-docker --quiet

# Vérifier les fichiers nécessaires
echo "📋 Vérification des fichiers..."
required_files=("requirements-no-psutil.txt" "web/api/server.py" "web/index.html")
for file in "${required_files[@]}"; do
    if [ ! -f "$file" ] && [ ! -d "$file" ]; then
        echo "❌ Fichier manquant: $file"
        exit 1
    fi
done

echo "✅ Tous les fichiers nécessaires sont présents"

# Créer un Dockerfile temporaire optimisé si nécessaire
if [ ! -f "Dockerfile" ]; then
    echo "📄 Création d'un Dockerfile temporaire..."
    cat > Dockerfile << 'EOF'
FROM python:3.11-slim

WORKDIR /app

# Copier et installer les dépendances
COPY requirements-no-psutil.txt requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Copier le code de l'application
COPY web/ ./web/
COPY biological_bitcoin_learning.py ./
COPY real_mea_interface.py ./
COPY src/ ./src/
COPY config/ ./config/

# Créer les répertoires nécessaires
RUN mkdir -p /app/data /app/uploads /app/logs

# Variables d'environnement
ENV PORT=8080
ENV PYTHONPATH=/app
ENV NODE_ENV=production

EXPOSE 8080

CMD ["python", "-m", "uvicorn", "web.api.server:app", "--host", "0.0.0.0", "--port", "8080"]
EOF
    TEMP_DOCKERFILE=true
else
    echo "✅ Utilisation du Dockerfile existant"
    TEMP_DOCKERFILE=false
fi

# Déployer depuis les sources
echo "🚀 Déploiement depuis les sources..."

gcloud run deploy "${SERVICE_NAME}" \
    --source . \
    --platform managed \
    --region "${REGION}" \
    --project "${PROJECT_ID}" \
    --memory "2Gi" \
    --cpu "2" \
    --concurrency "50" \
    --timeout "600s" \
    --min-instances "0" \
    --max-instances "10" \
    --port 8080 \
    --allow-unauthenticated \
    --ingress all \
    --execution-environment gen2 \
    --set-env-vars "PORT=8080" \
    --set-env-vars "NODE_ENV=production" \
    --set-env-vars "BIOMINING_ENVIRONMENT=production" \
    --set-env-vars "GOOGLE_CLOUD_PROJECT=${PROJECT_ID}" \
    --labels "app=biomining-web,version=1-0-0,environment=production"

deployment_success=$?

# Nettoyer le Dockerfile temporaire
if [ "$TEMP_DOCKERFILE" = true ]; then
    rm -f Dockerfile
    echo "🧹 Dockerfile temporaire supprimé"
fi

# Vérifier le succès du déploiement
if [ $deployment_success -eq 0 ]; then
    echo ""
    echo "🎉 DEPLOYMENT SUCCESSFUL!"
    echo "========================="
    
    # Récupérer l'URL du service
    SERVICE_URL=$(gcloud run services describe "${SERVICE_NAME}" \
        --region "${REGION}" \
        --project "${PROJECT_ID}" \
        --format "value(status.url)")
    
    echo "🌐 Service URL: ${SERVICE_URL}"
    echo ""
    echo "🔗 URLs disponibles:"
    echo "   🏠 Interface principale: ${SERVICE_URL}/"
    echo "   📊 API Status:          ${SERVICE_URL}/api/status"
    echo "   🔌 WebSocket:           wss://${SERVICE_URL#https://}/ws/hybrid-mining"
    echo ""
    echo "🧠⚡ Votre interface révolutionnaire de minage Bitcoin biologique est en ligne! ⚡🧠"
    echo ""

    # Tester le déploiement
    echo "🧪 Test de l'application..."
    sleep 5  # Attendre que le service démarre
    
    if curl -f -s "${SERVICE_URL}/api/status" > /dev/null; then
        echo "✅ Test de santé réussi!"
    else
        echo "⚠️  Test de santé échoué - le service démarre peut-être encore"
        echo "   Attendez 30-60 secondes et testez l'URL manuellement"
    fi

    echo ""
    echo "📱 Déploiement terminé! Visitez l'URL ci-dessus pour accéder à votre interface."
else
    echo ""
    echo "❌ ÉCHEC DU DÉPLOIEMENT"
    echo "======================="
    echo "💡 Conseils de dépannage:"
    echo "   1. Vérifiez les logs: gcloud logging read 'resource.type=\"cloud_run_revision\"' --limit 20"
    echo "   2. Vérifiez les quotas: gcloud compute project-info describe --project=${PROJECT_ID}"
    echo "   3. Essayez avec le script alternatif: ./deploy_no_psutil.sh"
    exit 1
fi