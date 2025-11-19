#!/bin/bash

# Script to check Google Cloud Run logs for the biomining platform

SERVICE_NAME="biomining-platform"
REGION="us-central1"

echo "🔍 Fetching recent logs from Cloud Run..."
echo ""

# Get logs from last 10 minutes
gcloud run logs read "$SERVICE_NAME" \
    --region="$REGION" \
    --limit=100 \
    --format="table(timestamp,severity,textPayload)" \
    2>&1

echo ""
echo "🔍 Searching for errors..."
gcloud run logs read "$SERVICE_NAME" \
    --region="$REGION" \
    --limit=200 \
    2>&1 | grep -i "error\|traceback\|exception\|training\|import" | head -30
