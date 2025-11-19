#!/bin/bash

# Deploy BioMining Platform - Pure Python Version
# Deploys the complete platform with Real Bitcoin Mining to Google Cloud Run

set -e

# Configuration
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-$(gcloud config get-value project)}"
SERVICE_NAME="biomining-platform"
REGION="us-central1"
IMAGE_NAME="gcr.io/$PROJECT_ID/$SERVICE_NAME:latest"

echo "🚀 Deploying BioMining Platform (Pure Python Version)"
echo "📋 Configuration:"
echo "   Project ID: $PROJECT_ID"
echo "   Service: $SERVICE_NAME" 
echo "   Region: $REGION"
echo "   Image: $IMAGE_NAME"
echo ""

# Verify required files exist
echo "🔍 Verifying required files..."
required_files=(
    "Dockerfile"
    "requirements.txt"
    "web/api/server.py"
    "web/index.html"
    "real_bitcoin_miner.py"
    "stratum_client.py"
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
echo "🔧 Building with Cloud Build..."

# Build with Cloud Build
gcloud builds submit \
    --project="$PROJECT_ID" \
    --tag="$IMAGE_NAME" \
    --timeout=3600s \
    --machine-type=e2-highcpu-8 \
    .

BUILD_SUCCESS=$?

if [[ $BUILD_SUCCESS -eq 0 ]]; then
    echo "✅ Cloud Build successful"
else
    echo "❌ Cloud Build failed"
    exit 1
fi

echo ""
echo "🚀 Deploying to Cloud Run..."

# Deploy to Cloud Run
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
    --set-env-vars "BIOMINING_ENVIRONMENT=production,PYTHONUNBUFFERED=1" \
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
    
    # Wait for service to be ready
    echo "⏳ Waiting for service to be ready (90 seconds)..."
    sleep 90
    
    # Test the deployment
    echo "🧪 Testing deployment..."
    
    echo "📊 Testing status endpoint..."
    if curl -f "$SERVICE_URL/api/status" -s > /dev/null; then
        echo "✅ Status endpoint responding"
    else
        echo "⚠️ Status endpoint not responding yet"
    fi
    
else
    echo "❌ Could not retrieve service URL"
fi

echo ""
echo "📋 Deployment Summary:"
echo "   Service: $SERVICE_NAME"
echo "   URL: ${SERVICE_URL:-'Not available'}"
echo "   Platform: Pure Python (Real Bitcoin Mining enabled)"
echo "   Status: Deployed"
echo ""

if [[ -n "$SERVICE_URL" ]]; then
    echo "🔗 Access your BioMining Platform:"
    echo "   🌐 Web Interface: $SERVICE_URL"
    echo "   📊 API Status: $SERVICE_URL/api/status"
    echo "   ⛏️ Real Mining: Navigate to 'Real Bitcoin Mining' section"
    echo ""
    echo "✨ Features Available:"
    echo "   • Bio-Entropy Mining"
    echo "   • Hybrid Mining System"
    echo "   • Real Bitcoin Mining (Testnet/Mainnet)"
    echo "   • Historical Bitcoin Training"
    echo "   • Neural Network Training"
fi

echo ""
echo "✅ Deployment completed successfully!"
echo ""
echo "🛡️ Platform Features:"
echo "   - Pure Python implementation (no C++ dependencies)"
echo "   - Real Bitcoin mining with Stratum protocol"
echo "   - Bio-entropy enhanced nonce predictions"
echo "   - Multi-pool support with automatic failover"
echo "   - Real-time statistics and monitoring"
echo "   - WebSocket support for live updates"
