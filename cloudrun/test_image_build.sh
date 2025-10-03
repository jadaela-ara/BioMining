#!/bin/bash
# Test script to verify image tag construction

set -e

# Source deploy script variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
APP_NAME="biomining-platform"
BUILD_TAG="latest"
REGISTRY="gcr.io"
PROJECT_ID="${GOOGLE_CLOUD_PROJECT:-test-project}"

# Test image tag construction
IMAGE_TAG="${REGISTRY}/${PROJECT_ID}/${APP_NAME}:${BUILD_TAG}"

echo "=== IMAGE TAG TEST ==="
echo "Registry: $REGISTRY"
echo "Project ID: $PROJECT_ID"
echo "App Name: $APP_NAME"
echo "Build Tag: $BUILD_TAG"
echo ""
echo "Final Image Tag: $IMAGE_TAG"
echo ""
echo "Length: ${#IMAGE_TAG}"
echo "Valid format check:"

# Validate image tag format
if [[ $IMAGE_TAG =~ ^[a-zA-Z0-9][a-zA-Z0-9._-]*[a-zA-Z0-9]*(/[a-zA-Z0-9][a-zA-Z0-9._-]*[a-zA-Z0-9]*)*:[a-zA-Z0-9._-]+$ ]]; then
    echo "✅ Image tag format is VALID"
else
    echo "❌ Image tag format is INVALID"
fi

# Test with your actual project ID
if [ -n "$GOOGLE_CLOUD_PROJECT" ]; then
    echo ""
    echo "=== WITH ACTUAL PROJECT ==="
    REAL_IMAGE_TAG="${REGISTRY}/${GOOGLE_CLOUD_PROJECT}/${APP_NAME}:${BUILD_TAG}"
    echo "Real Image Tag: $REAL_IMAGE_TAG"
    
    if [[ $REAL_IMAGE_TAG =~ ^[a-zA-Z0-9][a-zA-Z0-9._-]*[a-zA-Z0-9]*(/[a-zA-Z0-9][a-zA-Z0-9._-]*[a-zA-Z0-9]*)*:[a-zA-Z0-9._-]+$ ]]; then
        echo "✅ Real image tag format is VALID"
    else
        echo "❌ Real image tag format is INVALID"
    fi
fi