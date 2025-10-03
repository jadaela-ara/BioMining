#!/bin/bash
# BioMining Platform - Déploiement Google Cloud Run
# Script automatisé pour build et déploiement

set -euo pipefail

# ====================================================================
# CONFIGURATION
# ====================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
APP_NAME="biomining-platform"
SERVICE_NAME="biomining"
REGION="${GOOGLE_CLOUD_REGION:-europe-west1}"
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-}"

# Configuration Cloud Run
MEMORY="2Gi"
CPU="2"
CONCURRENCY="100" 
TIMEOUT="300s"
MIN_INSTANCES="0"
MAX_INSTANCES="10"

# Build configuration
BUILD_TAG="latest"
REGISTRY="gcr.io"
DOCKERFILE="Dockerfile.simple"  # Use simple dockerfile by default
IMAGE_TAG=""  # Will be set during build

echo "🧬 === BIOMINING PLATFORM CLOUD RUN DEPLOYMENT ==="
echo "=================================================="
echo "Project ID: ${PROJECT_ID}"
echo "Region: ${REGION}"
echo "Service Name: ${SERVICE_NAME}"
echo "Memory: ${MEMORY}"
echo "CPU: ${CPU}"
echo "Dockerfile: ${DOCKERFILE}"
echo "=================================================="

# ====================================================================
# VÉRIFICATIONS PRÉALABLES
# ====================================================================

check_prerequisites() {
    echo "🔍 Vérification des prérequis..."
    
    # Vérifier gcloud CLI
    if ! command -v gcloud &> /dev/null; then
        echo "❌ gcloud CLI non trouvé. Veuillez l'installer:"
        echo "   https://cloud.google.com/sdk/docs/install"
        exit 1
    fi
    
    # Vérifier Docker
    if ! command -v docker &> /dev/null; then
        echo "❌ Docker non trouvé. Veuillez l'installer:"
        echo "   https://docs.docker.com/get-docker/"
        exit 1
    fi
    
    # Vérifier PROJECT_ID
    if [ -z "$PROJECT_ID" ]; then
        echo "❌ GOOGLE_CLOUD_PROJECT non défini"
        echo "   Exécutez: export GOOGLE_CLOUD_PROJECT=your-project-id"
        exit 1
    fi
    
    # Vérifier authentification
    if ! gcloud auth list --filter=status:ACTIVE --format="value(account)" | head -n1 > /dev/null; then
        echo "❌ Authentification Google Cloud requise"
        echo "   Exécutez: gcloud auth login"
        exit 1
    fi
    
    # Vérifier que le projet existe
    if ! gcloud projects describe "$PROJECT_ID" &> /dev/null; then
        echo "❌ Projet $PROJECT_ID non accessible"
        exit 1
    fi
    
    echo "✅ Tous les prérequis sont satisfaits"
}

# ====================================================================
# ACTIVATION DES SERVICES
# ====================================================================

enable_services() {
    echo "🔧 Activation des services Google Cloud..."
    
    local services=(
        "cloudbuild.googleapis.com"
        "run.googleapis.com"
        "containerregistry.googleapis.com"
        "artifactregistry.googleapis.com"
    )
    
    for service in "${services[@]}"; do
        echo "   Activating $service..."
        gcloud services enable "$service" --project="$PROJECT_ID"
    done
    
    echo "✅ Services activés"
}

# ====================================================================
# BUILD ET PUSH DE L'IMAGE DOCKER
# ====================================================================

build_and_push_image() {
    echo "🐳 Build de l'image Docker..."
    
    # Définir l'image tag globalement
    IMAGE_TAG="${REGISTRY}/${PROJECT_ID}/${APP_NAME}:${BUILD_TAG}"
    
    # Configurer Docker pour utiliser gcloud comme helper
    gcloud auth configure-docker --quiet
    
    # Build de l'image avec BuildKit pour optimisation
    export DOCKER_BUILDKIT=1
    
    echo "   Building image: $IMAGE_TAG (using $DOCKERFILE)"
    docker build \
        --platform linux/amd64 \
        --file "$PROJECT_DIR/$DOCKERFILE" \
        --tag "$IMAGE_TAG" \
        --build-arg BUILD_TYPE=Release \
        --build-arg GOOGLE_CLOUD_PROJECT="$PROJECT_ID" \
        "$PROJECT_DIR"
    
    # Push de l'image
    echo "   Pushing image: $IMAGE_TAG"
    docker push "$IMAGE_TAG"
    
    echo "✅ Image Docker buildée et pushée: $IMAGE_TAG"
}

# ====================================================================
# DÉPLOIEMENT CLOUD RUN
# ====================================================================

deploy_to_cloud_run() {
    local image_tag="$IMAGE_TAG"
    
    echo "☁️  Déploiement vers Cloud Run..."

    echo "=================================================="
echo "Project ID: ${PROJECT_ID}"
echo "Region: ${REGION}"
echo "Service Name: ${SERVICE_NAME}"
echo "Memory: ${MEMORY}"
echo "CPU: ${CPU}"
echo "Dockerfile: ${DOCKERFILE}"
echo "Image Tag: $image_tag"
echo "=================================================="
    
    # Variables d'environnement pour l'application
    local env_vars=(
        "BIOMINING_ENVIRONMENT=production"
        "GOOGLE_CLOUD_PROJECT=${PROJECT_ID}"
        "DEPLOYMENT_TIMESTAMP=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
        "BIOMINING_LOG_LEVEL=info"
    )
    
    # Construire la commande gcloud run deploy
    local deploy_cmd=(
        gcloud run deploy "$SERVICE_NAME"
        --image "$image_tag"
        --platform managed
        --region "$REGION"
        --project "$PROJECT_ID"
        --memory "$MEMORY"
        --cpu "$CPU"
        --concurrency "$CONCURRENCY"
        --timeout "$TIMEOUT"
        --min-instances "$MIN_INSTANCES"
        --max-instances "$MAX_INSTANCES"
        --port 8080
        --allow-unauthenticated
        --ingress all
        --execution-environment gen2
    )
    
    # Ajouter les variables d'environnement
    for env_var in "${env_vars[@]}"; do
        deploy_cmd+=(--set-env-vars "$env_var")
    done
    
    # Labels pour organisation
    deploy_cmd+=(
        --labels "app=biomining,version=1.0.0,environment=production"
        --tag "latest"
    )
    
    echo "   Executing deployment..."
    "${deploy_cmd[@]}"
    
    # Récupérer l'URL du service
    local service_url
    service_url=$(gcloud run services describe "$SERVICE_NAME" \
        --region "$REGION" \
        --project "$PROJECT_ID" \
        --format "value(status.url)")
    
    echo "✅ Déploiement Cloud Run terminé"
    echo "🌐 Service URL: $service_url"
    
    return 0
}

# ====================================================================
# TESTS POST-DÉPLOIEMENT
# ====================================================================

run_health_checks() {
    local service_url="$1"
    
    echo "🏥 Tests de santé post-déploiement..."
    
    # Attendre que le service soit disponible
    echo "   Attente de la disponibilité du service..."
    sleep 30
    
    # Test de health check
    echo "   Test health check..."
    if curl -f -s "${service_url}/health" > /dev/null; then
        echo "✅ Health check: OK"
    else
        echo "❌ Health check: ÉCHEC"
        return 1
    fi
    
    # Test API status
    echo "   Test API status..."
    if curl -f -s "${service_url}/api/status" > /dev/null; then
        echo "✅ API status: OK"
    else
        echo "❌ API status: ÉCHEC"
        return 1
    fi
    
    # Test performances basique
    echo "   Test performance..."
    local response_time
    response_time=$(curl -w "%{time_total}" -s -o /dev/null "${service_url}/health")
    
    if (( $(echo "$response_time < 2.0" | bc -l) )); then
        echo "✅ Performance: OK (${response_time}s)"
    else
        echo "⚠️  Performance: LENT (${response_time}s)"
    fi
    
    echo "✅ Tous les tests de santé réussis"
}

# ====================================================================
# GESTION DU TRAFIC
# ====================================================================

manage_traffic() {
    local service_name="$1"
    
    echo "🚦 Gestion du trafic..."
    
    # Diriger 100% du trafic vers la nouvelle révision
    gcloud run services update-traffic "$service_name" \
        --to-latest \
        --region "$REGION" \
        --project "$PROJECT_ID"
    
    echo "✅ Trafic dirigé vers la nouvelle révision"
}

# ====================================================================
# FONCTION PRINCIPALE
# ====================================================================

main() {
    echo "🚀 Début du déploiement BioMining Platform"
    
    # Vérifications
    check_prerequisites
    
    # Activer les services nécessaires
    enable_services
    
    # Build et push de l'image
    build_and_push_image
    
    # Déploiement
    deploy_to_cloud_run
    
    # Récupération de l'URL du service
    local service_url
    service_url=$(gcloud run services describe "$SERVICE_NAME" \
        --region "$REGION" \
        --project "$PROJECT_ID" \
        --format "value(status.url)")
    
    # Gestion du trafic
    manage_traffic "$SERVICE_NAME"
    
    # Tests post-déploiement
    run_health_checks "$service_url"
    
    echo ""
    echo "🎉 DÉPLOIEMENT TERMINÉ AVEC SUCCÈS!"
    echo "=========================================="
    echo "🌐 Service URL: $service_url"
    echo "📊 Monitoring: https://console.cloud.google.com/run/detail/${REGION}/${SERVICE_NAME}"
    echo "📈 Métriques: ${service_url}/api/metrics"
    echo "🏥 Health Check: ${service_url}/health"
    echo ""
    echo "🔗 Endpoints disponibles:"
    echo "   GET  ${service_url}/api/status"
    echo "   POST ${service_url}/api/configure"
    echo "   GET  ${service_url}/api/metrics"
    echo "   POST ${service_url}/api/train/start"
    echo "   POST ${service_url}/api/mine/start"
    echo "   GET  ${service_url}/api/export"
    echo ""
    echo "🧬 BioMining Platform déployée et opérationnelle!"
}

# ====================================================================
# GESTION DES OPTIONS
# ====================================================================

show_help() {
    cat << EOF
BioMining Platform - Déploiement Cloud Run

Usage: $0 [OPTIONS]

Options:
  --help          Afficher cette aide
  --project ID    Google Cloud Project ID
  --region REGION Région Cloud Run (défaut: us-central1)
  --service NAME  Nom du service (défaut: biomining)
  --memory SIZE   Mémoire allouée (défaut: 2Gi)
  --cpu COUNT     CPU alloués (défaut: 2)
  --dockerfile NAME Dockerfile à utiliser (défaut: Dockerfile.simple)

Variables d'environnement:
  GOOGLE_CLOUD_PROJECT  Project ID (requis)
  GOOGLE_CLOUD_REGION   Région (optionnel)

Exemples:
  $0
  $0 --project my-project --region europe-west1
  $0 --dockerfile Dockerfile  # Use advanced dockerfile
  $0 --dockerfile Dockerfile.simple  # Use simple dockerfile
  
EOF
}

# Parse des arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --help)
            show_help
            exit 0
            ;;
        --project)
            PROJECT_ID="$2"
            shift 2
            ;;
        --region)
            REGION="$2"
            shift 2
            ;;
        --service)
            SERVICE_NAME="$2"
            shift 2
            ;;
        --memory)
            MEMORY="$2"
            shift 2
            ;;
        --cpu)
            CPU="$2"
            shift 2
            ;;
        --dockerfile)
            DOCKERFILE="$2"
            shift 2
            ;;
        *)
            echo "Option inconnue: $1"
            show_help
            exit 1
            ;;
    esac
done

# Lancement du déploiement
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
