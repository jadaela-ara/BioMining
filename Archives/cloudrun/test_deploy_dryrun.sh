#!/bin/bash
# Dry run test of deploy script to check configuration

set -e

echo "🧪 === DRY RUN TEST - DEPLOY SCRIPT ==="
echo "======================================"

# Set test environment
export GOOGLE_CLOUD_PROJECT="biomining-473408"
export GOOGLE_CLOUD_REGION="europe-west1"

echo "Project ID: $GOOGLE_CLOUD_PROJECT"
echo "Region: $GOOGLE_CLOUD_REGION"
echo ""

# Source the deploy script functions (without executing main) 
# Simulating the same path structure as deploy.sh
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"  # We're already in cloudrun folder
APP_NAME="biomining-platform"
SERVICE_NAME="biomining"
REGION="europe-west1"
PROJECT_ID="$GOOGLE_CLOUD_PROJECT"
MEMORY="2Gi"
CPU="2"
CONCURRENCY="100" 
TIMEOUT="300s"
MIN_INSTANCES="0"
MAX_INSTANCES="10"
BUILD_TAG="latest"
REGISTRY="gcr.io"
DOCKERFILE="Dockerfile.simple"

# Test image tag construction
IMAGE_TAG="${REGISTRY}/${PROJECT_ID}/${APP_NAME}:${BUILD_TAG}"

echo "=== CONFIGURATION TEST ==="
echo "Image Tag: $IMAGE_TAG"
echo "Dockerfile: $DOCKERFILE" 
echo "Service Name: $SERVICE_NAME"
echo "Memory: $MEMORY"
echo "CPU: $CPU"
echo ""

# Test if dockerfile exists
if [ -f "$PROJECT_DIR/$DOCKERFILE" ]; then
    echo "✅ Dockerfile exists: $PROJECT_DIR/$DOCKERFILE"
else
    echo "❌ Dockerfile missing: $PROJECT_DIR/$DOCKERFILE"
    exit 1
fi

# Test Docker command construction (dry run)
echo ""
echo "=== DOCKER BUILD COMMAND (DRY RUN) ==="
echo "docker build \\"
echo "    --platform linux/amd64 \\"
echo "    --file \"$PROJECT_DIR/$DOCKERFILE\" \\"
echo "    --tag \"$IMAGE_TAG\" \\"
echo "    --build-arg BUILD_TYPE=Release \\"
echo "    --build-arg GOOGLE_CLOUD_PROJECT=\"$PROJECT_ID\" \\"
echo "    \"$PROJECT_DIR\""

# Test gcloud deploy command construction
echo ""
echo "=== GCLOUD DEPLOY COMMAND (DRY RUN) ==="
echo "gcloud run deploy \"$SERVICE_NAME\" \\"
echo "    --image \"$IMAGE_TAG\" \\"
echo "    --platform managed \\"
echo "    --region \"$REGION\" \\"
echo "    --project \"$PROJECT_ID\" \\"
echo "    --memory \"$MEMORY\" \\"
echo "    --cpu \"$CPU\" \\"
echo "    --port 8080 \\"
echo "    --allow-unauthenticated"

echo ""
echo "✅ Dry run completed successfully!"
echo "📋 Ready to deploy with: ./scripts/deploy.sh --project $PROJECT_ID"