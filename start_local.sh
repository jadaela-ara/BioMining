#!/bin/bash
# ================================================================
# BioMining Platform - Local Deployment Script
# ================================================================
# This script starts the BioMining platform locally
# No Docker, no Cloud Run, no costs!
# ================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================================${NC}"
echo -e "${BLUE}🚀 BioMining Platform - Local Deployment${NC}"
echo -e "${BLUE}================================================================${NC}"
echo ""

# Check if we're in the right directory
if [ ! -f "requirements.txt" ]; then
    echo -e "${RED}❌ Error: requirements.txt not found${NC}"
    echo -e "${YELLOW}Please run this script from the BioMining root directory${NC}"
    exit 1
fi

# Check Python version
echo -e "${BLUE}📋 Checking Python version...${NC}"
PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
echo -e "${GREEN}✅ Python $PYTHON_VERSION${NC}"

# Check if virtual environment exists
if [ ! -d "venv" ]; then
    echo -e "${YELLOW}⚠️  Virtual environment not found${NC}"
    echo -e "${BLUE}Creating virtual environment...${NC}"
    python3 -m venv venv
    echo -e "${GREEN}✅ Virtual environment created${NC}"
fi

# Activate virtual environment
echo -e "${BLUE}🔧 Activating virtual environment...${NC}"
source venv/bin/activate
echo -e "${GREEN}✅ Virtual environment activated${NC}"

# Install/update dependencies
echo -e "${BLUE}📦 Installing dependencies...${NC}"
pip install -q -r requirements.txt
echo -e "${GREEN}✅ Dependencies installed${NC}"

# Get local IP address
LOCAL_IP=$(hostname -I | awk '{print $1}' 2>/dev/null || echo "localhost")

# Display information
echo ""
echo -e "${GREEN}================================================================${NC}"
echo -e "${GREEN}🎉 BioMining Platform Ready!${NC}"
echo -e "${GREEN}================================================================${NC}"
echo ""
echo -e "${YELLOW}📍 Access URLs:${NC}"
echo -e "   Local:    ${GREEN}http://localhost:8000${NC}"
echo -e "   Network:  ${GREEN}http://${LOCAL_IP}:8000${NC}"
echo ""
echo -e "${YELLOW}📊 API Endpoints:${NC}"
echo -e "   Health:   ${GREEN}http://localhost:8000/api/health${NC}"
echo -e "   Training: ${GREEN}http://localhost:8000/api/training/historical/status${NC}"
echo ""
echo -e "${YELLOW}🛑 To stop the server:${NC} Press ${RED}Ctrl + C${NC}"
echo ""
echo -e "${BLUE}================================================================${NC}"
echo -e "${BLUE}🚀 Starting server...${NC}"
echo -e "${BLUE}================================================================${NC}"
echo ""

# Start the server
python -m uvicorn web.api.server:app --host 0.0.0.0 --port 8000 --reload
