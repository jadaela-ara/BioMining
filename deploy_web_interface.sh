#!/bin/bash
# ================================================================
# QUICK DEPLOY - WEB INTERFACE TO CLOUDRUN
# Deploy directly from feature branch with web interface
# ================================================================

set -euo pipefail

echo "🚀 DEPLOYING HYBRID BITCOIN MINING WEB INTERFACE TO CLOUDRUN"
echo "============================================================="

# Configuration
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-}"
REGION="${GOOGLE_CLOUD_REGION:-europe-west1}"
SERVICE_NAME="biomining-web"
BRANCH="feature/real-mea-interface"

# Check prerequisites
if [ -z "$PROJECT_ID" ]; then
    echo "❌ GOOGLE_CLOUD_PROJECT environment variable required"
    echo "   Set it with: export GOOGLE_CLOUD_PROJECT=your-project-id"
    exit 1
fi

if ! command -v gcloud &> /dev/null; then
    echo "❌ gcloud CLI not found. Install it from:"
    echo "   https://cloud.google.com/sdk/docs/install"
    exit 1
fi

echo "✅ Project ID: ${PROJECT_ID}"
echo "✅ Region: ${REGION}"
echo "✅ Service: ${SERVICE_NAME}"
echo "✅ Branch: ${BRANCH}"
echo ""

# Enable required services
echo "🔧 Enabling required Google Cloud services..."
gcloud services enable cloudbuild.googleapis.com --project="${PROJECT_ID}"
gcloud services enable run.googleapis.com --project="${PROJECT_ID}"
gcloud services enable containerregistry.googleapis.com --project="${PROJECT_ID}"

# Configure Docker auth
gcloud auth configure-docker --quiet

# Deploy from source (CloudRun will pull from Git)
echo "🚀 Deploying from source..."

gcloud run deploy "${SERVICE_NAME}" \
    --source . \
    --platform managed \
    --region "${REGION}" \
    --project "${PROJECT_ID}" \
    --memory "4Gi" \
    --cpu "4" \
    --concurrency "50" \
    --timeout "600s" \
    --min-instances "0" \
    --max-instances "5" \
    --port 8080 \
    --allow-unauthenticated \
    --ingress all \
    --execution-environment gen2 \
    --set-env-vars "BIOMINING_ENVIRONMENT=production" \
    --set-env-vars "GOOGLE_CLOUD_PROJECT=${PROJECT_ID}" \
    --set-env-vars "ENABLE_TRIPLE_SYSTEM=true" \
    --set-env-vars "ENABLE_BIOLOGICAL_NETWORK=true" \
    --set-env-vars "ENABLE_REAL_MEA=true" \
    --set-env-vars "ENABLE_CROSS_LEARNING=true" \
    --set-env-vars "MEA_ELECTRODE_COUNT=60" \
    --set-env-vars "WEBSOCKET_ENABLED=true" \
    --set-env-vars "REALTIME_MONITORING=true" \
    --set-env-vars "NODE_ENV=production" \
    --labels "app=biomining-web,version=1-0-0,environment=production"

# Get service URL
SERVICE_URL=$(gcloud run services describe "${SERVICE_NAME}" \
    --region "${REGION}" \
    --project "${PROJECT_ID}" \
    --format "value(status.url)")

echo ""
echo "🎉 DEPLOYMENT SUCCESSFUL!"
echo "========================="
echo "🌐 Service URL: ${SERVICE_URL}"
echo ""
echo "🔗 Available Endpoints:"
echo "   🏠 Main Interface:     ${SERVICE_URL}/"
echo "   📊 API Status:         ${SERVICE_URL}/api/status"
echo "   🔌 WebSocket:          wss://${SERVICE_URL#https://}/ws/hybrid-mining"
echo ""
echo "🧠⚡ Your revolutionary biological Bitcoin mining interface is live! ⚡🧠"
echo ""

# Test the deployment
echo "🧪 Testing deployment..."
if curl -f -s "${SERVICE_URL}/api/status" > /dev/null; then
    echo "✅ Health check passed!"
else
    echo "⚠️  Health check failed - service may still be starting up"
fi

echo ""
echo "🚀 Deployment complete! Visit the URL above to access your interface."