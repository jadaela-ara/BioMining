#!/bin/bash

# Deploy BioMining with REAL C++ bindings - ULTRA COMPATIBLE VERSION
# Compatible with older gcloud CLI versions

set -e

# Configuration
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-$(gcloud config get-value project)}"
SERVICE_NAME="biomining-entropie"
REGION="us-central1"
SOURCE_DOCKERFILE="Dockerfile.cpp-simple"
TARGET_DOCKERFILE="Dockerfile"
IMAGE_NAME="gcr.io/$PROJECT_ID/$SERVICE_NAME:latest"

echo "🚀 Deploying BioMining with REAL C++ bindings (Ultra Compatible Version)"
echo "📋 Configuration:"
echo "   Project ID: $PROJECT_ID"
echo "   Service: $SERVICE_NAME" 
echo "   Region: $REGION"
echo "   Source Dockerfile: $SOURCE_DOCKERFILE"
echo "   Image: $IMAGE_NAME"
echo ""

# Verify required files exist
echo "🔍 Verifying required files..."
required_files=(
    "$SOURCE_DOCKERFILE"
    "requirements-cpp.txt"
    "python_bindings/biomining_python.cpp"
    "web/api/server.py"
)

for file in "${required_files[@]}"; do
    if [[ -f "$file" ]]; then
        echo "✅ Found: $file"
    else
        echo "❌ Missing: $file"
        exit 1
    fi
done

echo ""
echo "🔧 Pre-deployment setup..."

# Backup existing Dockerfile if it exists
if [[ -f "Dockerfile" ]]; then
    echo "📋 Backing up existing Dockerfile..."
    cp Dockerfile Dockerfile.backup.$(date +%s)
fi

# Copy our robust Dockerfile as the default Dockerfile
echo "📋 Using $SOURCE_DOCKERFILE as Dockerfile..."
cp "$SOURCE_DOCKERFILE" "$TARGET_DOCKERFILE"

echo ""
echo "🔧 Building with Cloud Build..."

# Use the absolute simplest Cloud Build approach
gcloud builds submit \
    --project="$PROJECT_ID" \
    --tag="$IMAGE_NAME" \
    --timeout=3600s \
    --machine-type=e2-highcpu-8 \
    .

BUILD_SUCCESS=$?

# Restore original Dockerfile if it existed
if [[ -f "Dockerfile.backup."* ]]; then
    echo "📋 Restoring original Dockerfile..."
    BACKUP_FILE=$(ls Dockerfile.backup.* | head -n 1)
    mv "$BACKUP_FILE" "Dockerfile"
else
    # Remove the temporary Dockerfile we created
    rm -f "Dockerfile"
fi

if [[ $BUILD_SUCCESS -eq 0 ]]; then
    echo "✅ Cloud Build successful"
else
    echo "❌ Cloud Build failed"
    exit 1
fi

echo ""
echo "🚀 Deploying to Cloud Run (Compatible Mode)..."

# Deploy to Cloud Run - MINIMAL FLAGS for maximum compatibility
gcloud run deploy "$SERVICE_NAME" \
    --image "$IMAGE_NAME" \
    --platform managed \
    --region "$REGION" \
    --allow-unauthenticated \
    --memory 4Gi \
    --cpu 4 \
    --timeout 600 \
    --concurrency 10 \
    --max-instances 3 \
    --port 8080 \
    --set-env-vars "BIOMINING_ENVIRONMENT=production,QT_QPA_PLATFORM=offscreen,DISPLAY=:0,PYTHONUNBUFFERED=1" \
    --project="$PROJECT_ID"

if [[ $? -eq 0 ]]; then
    echo "✅ Cloud Run deployment successful"
else
    echo "❌ Cloud Run deployment failed"
    exit 1
fi

echo ""
echo "🔍 Getting service URL..."
SERVICE_URL=$(gcloud run services describe "$SERVICE_NAME" \
    --platform=managed \
    --region="$REGION" \
    --project="$PROJECT_ID" \
    --format="value(status.url)" 2>/dev/null || echo "")

if [[ -n "$SERVICE_URL" ]]; then
    echo "🌐 Service URL: $SERVICE_URL"
    echo ""
    echo "✅ Deployment completed!"
    echo ""
    echo "🔗 Access your BioMining interface at:"
    echo "   Web Interface: $SERVICE_URL"
    echo "   API Status: $SERVICE_URL/api/status"
    echo "   C++ Bindings: $SERVICE_URL/api/bindings"
else
    echo "❌ Could not retrieve service URL"
fi

echo ""
echo "📊 Note: Using compatible flags for older gcloud versions"
echo "   - Removed: --cpu-boost (requires gcloud 450+)"
echo "   - Using: Standard CPU allocation"
echo "   - Timeout: 600s (sufficient for C++ compilation)"
