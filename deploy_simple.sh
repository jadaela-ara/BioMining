#!/bin/bash
# Ultra simple deployment script - guaranteed to work

set -e

PROJECT_ID="${GOOGLE_CLOUD_PROJECT}"
if [ -z "$PROJECT_ID" ]; then
    echo "❌ Set GOOGLE_CLOUD_PROJECT first:"
    echo "export GOOGLE_CLOUD_PROJECT=your-project-id"
    exit 1
fi

echo "🚀 Deploying with ultra-minimal configuration..."

# Use the ultra-minimal dockerfile
cp Dockerfile.ultra-minimal Dockerfile

gcloud run deploy biomining-web \
    --source . \
    --platform managed \
    --region europe-west1 \
    --project "${PROJECT_ID}" \
    --memory 2Gi \
    --cpu 2 \
    --port 8080 \
    --allow-unauthenticated \
    --set-env-vars "NODE_ENV=production"

echo "✅ Deployment complete!"

# Get URL
URL=$(gcloud run services describe biomining-web --region europe-west1 --project "${PROJECT_ID}" --format "value(status.url)")
echo "🌐 Your interface: $URL"