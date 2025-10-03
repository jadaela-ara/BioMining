#!/bin/bash
# BioMining Platform - Test Build Local

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOCKERFILE="${1:-Dockerfile.simple}"

echo "🔧 Testing local build with $DOCKERFILE..."

cd "$SCRIPT_DIR"

# Build image locally
docker build -f "$DOCKERFILE" -t biomining:test .

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "🧪 Testing container..."
    
    # Start container in background
    CONTAINER_ID=$(docker run -d -p 8080:8080 biomining:test)
    
    # Wait a bit for startup
    sleep 10
    
    # Test health endpoint
    if curl -f -s http://localhost:8080/health; then
        echo "✅ Container is working!"
    else
        echo "❌ Container health check failed"
        docker logs "$CONTAINER_ID"
    fi
    
    # Cleanup
    docker stop "$CONTAINER_ID"
    docker rm "$CONTAINER_ID"
    
else
    echo "❌ Build failed!"
    exit 1
fi