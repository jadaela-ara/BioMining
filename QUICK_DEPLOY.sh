#!/bin/bash

# 🚀 QUICK DEPLOYMENT SCRIPT FOR CLOUD SHELL
# Copy and paste this entire script into Google Cloud Shell

set -e

echo "🚀 BioMining Quick Deployment Script"
echo "===================================="
echo ""

# Step 1: Clone or update repository
if [ -d "BioMining" ]; then
    echo "📁 Repository exists, updating..."
    cd BioMining
    git fetch origin
else
    echo "📥 Cloning repository..."
    git clone https://github.com/jadaela-ara/BioMining.git
    cd BioMining
fi

# Step 2: Checkout the branch with fixes
echo ""
echo "🔄 Checking out branch with fixes..."
git checkout genspark_ai_developer
git pull origin genspark_ai_developer

# Step 3: Verify patch is present
echo ""
echo "🔍 Verifying lazy initialization patch..."
if grep -q "def get_platform():" web/api/server.py; then
    echo "✅ Patch verified: Lazy initialization function found"
else
    echo "❌ ERROR: Patch not found in web/api/server.py"
    echo "Please ensure you're on the correct branch"
    exit 1
fi

# Step 4: Show what will be deployed
echo ""
echo "📋 Deployment Configuration:"
echo "   Branch: genspark_ai_developer"
echo "   Commit: $(git rev-parse --short HEAD)"
echo "   Dockerfile: Dockerfile.cpp-simple-safe"
echo "   Service: biomining-entropie"
echo "   Region: us-central1"
echo ""

# Step 5: Confirm deployment
echo "⚠️  This will start a Cloud Build (30-40 minutes) and deploy to Cloud Run"
read -p "Continue with deployment? (yes/no): " -r
echo ""

if [[ ! $REPLY =~ ^[Yy]es$ ]]; then
    echo "❌ Deployment cancelled"
    exit 0
fi

# Step 6: Deploy
echo "🚀 Starting deployment..."
echo ""
chmod +x deploy_cpp_safe.sh
./deploy_cpp_safe.sh

echo ""
echo "✅ Deployment script completed!"
