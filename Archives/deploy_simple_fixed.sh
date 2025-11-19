#!/bin/bash
# ================================================================
# DÉPLOIEMENT SIMPLE ET ROBUSTE - BIOMINING WEB INTERFACE
# Script de déploiement simplifié avec gestion d'erreurs améliorée
# ================================================================

set -euo pipefail

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Fonction d'affichage avec couleurs
log_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }
log_success() { echo -e "${GREEN}✅ $1${NC}"; }
log_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
log_error() { echo -e "${RED}❌ $1${NC}"; }

echo "🚀 DÉPLOIEMENT SIMPLE - BIOMINING WEB INTERFACE"
echo "=============================================="

# Configuration par défaut
DEFAULT_PROJECT_ID=""
DEFAULT_REGION="europe-west1"
SERVICE_NAME="biomining-web"

# Demander le PROJECT_ID si non configuré
get_project_id() {
    # Essayer de récupérer depuis l'environnement
    if [ -n "${GOOGLE_CLOUD_PROJECT:-}" ]; then
        PROJECT_ID="$GOOGLE_CLOUD_PROJECT"
        log_info "Utilisation du PROJECT_ID depuis GOOGLE_CLOUD_PROJECT: $PROJECT_ID"
        return
    fi
    
    # Essayer de récupérer depuis gcloud config
    PROJECT_ID=$(gcloud config get-value project 2>/dev/null || echo "")
    
    if [ -n "$PROJECT_ID" ]; then
        log_info "Utilisation du PROJECT_ID depuis gcloud config: $PROJECT_ID"
        return
    fi
    
    # Demander à l'utilisateur
    log_warning "Aucun projet GCP configuré."
    echo ""
    echo "📋 Projets disponibles :"
    gcloud projects list --format="table(projectId,name)" 2>/dev/null || {
        log_error "Impossible de lister les projets. Vous devez vous authentifier :"
        echo "   gcloud auth login"
        exit 1
    }
    echo ""
    read -p "Entrez votre PROJECT_ID : " PROJECT_ID
    
    if [ -z "$PROJECT_ID" ]; then
        log_error "PROJECT_ID requis pour le déploiement"
        exit 1
    fi
}

# Vérifications préliminaires
log_info "Vérifications préliminaires..."

# Vérifier gcloud CLI
if ! command -v gcloud &> /dev/null; then
    log_error "gcloud CLI non trouvé"
    echo "📥 Installez-le depuis: https://cloud.google.com/sdk/docs/install"
    exit 1
fi

# Vérifier curl pour les tests
if ! command -v curl &> /dev/null; then
    log_warning "curl non trouvé - les tests de santé seront ignorés"
fi

# Obtenir le PROJECT_ID
get_project_id

# Vérifier l'authentification
if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | grep -q .; then
    log_error "Vous n'êtes pas authentifié avec gcloud"
    echo "🔐 Exécutez: gcloud auth login"
    exit 1
fi

# Afficher la configuration
log_success "Configuration validée"
echo "📊 Paramètres de déploiement:"
echo "   - Projet GCP: $PROJECT_ID"
echo "   - Région: $DEFAULT_REGION"
echo "   - Service: $SERVICE_NAME"
echo ""

# Configurer le projet
log_info "Configuration du projet GCP..."
gcloud config set project "$PROJECT_ID"

# Activer les services nécessaires
log_info "Activation des services Google Cloud..."
gcloud services enable cloudbuild.googleapis.com --quiet
gcloud services enable run.googleapis.com --quiet
gcloud services enable containerregistry.googleapis.com --quiet

# Configurer Docker auth
log_info "Configuration de l'authentification Docker..."
gcloud auth configure-docker --quiet

# Vérifier les fichiers nécessaires
log_info "Vérification des fichiers du projet..."
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
    log_error "Fichiers manquants :"
    printf '   - %s\n' "${missing_files[@]}"
    exit 1
fi

log_success "Tous les fichiers nécessaires sont présents"

# Créer un Dockerfile optimisé temporaire
log_info "Préparation du conteneur..."
cat > Dockerfile.deploy << 'EOF'
FROM python:3.11-slim

WORKDIR /app

# Installer les dépendances système si nécessaires
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Copier et installer les dépendances Python
COPY requirements-no-psutil.txt requirements.txt
RUN pip install --no-cache-dir -r requirements.txt

# Copier le code de l'application
COPY web/ ./web/
COPY biological_bitcoin_learning.py ./
COPY real_mea_interface.py ./

# Copier les répertoires optionnels s'ils existent
COPY src/ ./src/ 2>/dev/null || echo "src/ directory not found, skipping"
COPY config/ ./config/ 2>/dev/null || echo "config/ directory not found, skipping"

# Créer les répertoires de données
RUN mkdir -p /app/data /app/uploads /app/logs

# Variables d'environnement
ENV PORT=8080
ENV PYTHONPATH=/app
ENV NODE_ENV=production
ENV BIOMINING_ENVIRONMENT=production

# Healthcheck
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
  CMD curl -f http://localhost:8080/api/status || exit 1

EXPOSE 8080

CMD ["python", "-m", "uvicorn", "web.api.server:app", "--host", "0.0.0.0", "--port", "8080", "--workers", "1"]
EOF

# Construire et déployer
log_info "Construction et déploiement de l'application..."
echo "⏳ Cette étape peut prendre 2-5 minutes..."

# Sauvegarder le Dockerfile existant s'il y en a un
if [ -f "Dockerfile" ]; then
    mv Dockerfile Dockerfile.backup
    BACKUP_NEEDED=true
else
    BACKUP_NEEDED=false
fi

# Utiliser notre Dockerfile personnalisé
mv Dockerfile.deploy Dockerfile

gcloud run deploy "$SERVICE_NAME" \
    --source . \
    --platform managed \
    --region "$DEFAULT_REGION" \
    --allow-unauthenticated \
    --memory "2Gi" \
    --cpu "2" \
    --concurrency "50" \
    --timeout "600s" \
    --max-instances "10" \
    --port 8080 \
    --set-env-vars "PORT=8080,NODE_ENV=production,BIOMINING_ENVIRONMENT=production" \
    --labels "app=biomining-web,version=1-0-0" \
    --quiet

# Restaurer le Dockerfile original
rm -f Dockerfile
if [ "$BACKUP_NEEDED" = true ]; then
    mv Dockerfile.backup Dockerfile
fi

deployment_exit_code=$?

# Le Dockerfile a déjà été nettoyé plus haut

# Vérifier le succès du déploiement
if [ $deployment_exit_code -eq 0 ]; then
    log_success "Déploiement terminé avec succès!"
    echo ""
    
    # Obtenir l'URL du service
    SERVICE_URL=$(gcloud run services describe "$SERVICE_NAME" \
        --region="$DEFAULT_REGION" \
        --format="value(status.url)")
    
    echo "🌐 VOTRE APPLICATION EST EN LIGNE !"
    echo "================================="
    echo "🔗 URL principale:     $SERVICE_URL/"
    echo "📊 API Status:         $SERVICE_URL/api/status"
    echo "🔌 WebSocket:          wss://${SERVICE_URL#https://}/ws/hybrid-mining"
    echo ""
    
    # Test de santé si curl est disponible
    if command -v curl &> /dev/null; then
        log_info "Test de l'application..."
        sleep 10  # Attendre que le service soit complètement démarré
        
        if curl -f -s "$SERVICE_URL/api/status" > /dev/null 2>&1; then
            log_success "Application en ligne et fonctionnelle!"
        else
            log_warning "L'application démarre encore. Testez dans 30-60 secondes."
        fi
    fi
    
    echo ""
    echo "🎉 Félicitations ! Votre interface de minage Bitcoin biologique est déployée !"
    echo "📱 Visitez l'URL ci-dessus pour accéder à votre application."
    
else
    log_error "Échec du déploiement"
    echo ""
    echo "🔍 Conseils de dépannage :"
    echo "   1. Vérifiez vos quotas GCP"
    echo "   2. Consultez les logs :"
    echo "      gcloud logging read 'resource.type=\"cloud_run_revision\"' --limit 10"
    echo "   3. Vérifiez la facturation du projet"
    echo "   4. Essayez avec une autre région :"
    echo "      GOOGLE_CLOUD_REGION=us-central1 $0"
    exit 1
fi