# 🚀 Deployment Guide - BioMining Platform (Pure Python)

## Overview

This guide covers deployment of the Pure Python BioMining Platform to Google Cloud Run.

## Prerequisites

- Google Cloud Platform account
- `gcloud` CLI installed and configured
- Project with billing enabled
- Necessary permissions for Cloud Run and Cloud Build

## Quick Deployment

### 1. Authenticate with Google Cloud

```bash
gcloud auth login
gcloud config set project YOUR_PROJECT_ID
```

### 2. Enable Required APIs

```bash
gcloud services enable cloudbuild.googleapis.com
gcloud services enable run.googleapis.com
gcloud services enable containerregistry.googleapis.com
```

### 3. Run Deployment Script

```bash
./deploy_python.sh
```

The script will:
- ✅ Verify all required files exist
- ✅ Build Docker image using Cloud Build
- ✅ Deploy to Cloud Run with optimal configuration
- ✅ Test the deployment endpoints
- ✅ Display the service URL

## Manual Deployment

### Build Docker Image

```bash
PROJECT_ID=$(gcloud config get-value project)
SERVICE_NAME="biomining-platform"
IMAGE_NAME="gcr.io/$PROJECT_ID/$SERVICE_NAME:latest"

gcloud builds submit \
  --project="$PROJECT_ID" \
  --tag="$IMAGE_NAME" \
  --timeout=3600s \
  --machine-type=e2-highcpu-8 \
  .
```

### Deploy to Cloud Run

```bash
gcloud run deploy "$SERVICE_NAME" \
  --image "$IMAGE_NAME" \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated \
  --memory 4Gi \
  --cpu 4 \
  --timeout 600 \
  --concurrency 10 \
  --max-instances 3 \
  --port 8080 \
  --set-env-vars "BIOMINING_ENVIRONMENT=production,PYTHONUNBUFFERED=1" \
  --project="$PROJECT_ID"
```

## Configuration

### Resource Allocation

- **Memory**: 4Gi (required for mining operations)
- **CPU**: 4 cores (optimal for parallel mining)
- **Timeout**: 600 seconds (10 minutes)
- **Concurrency**: 10 requests per instance
- **Max Instances**: 3 (cost control)

### Environment Variables

- `BIOMINING_ENVIRONMENT=production` - Sets production mode
- `PYTHONUNBUFFERED=1` - Enables real-time logging
- `PORT=8080` - Service port (configured in Dockerfile)

## Features Available Post-Deployment

### ✨ Core Features

1. **Bio-Entropy Mining**
   - Biological data enhanced mining
   - Advanced nonce prediction

2. **Real Bitcoin Mining**
   - Testnet support for testing
   - Mainnet support for production
   - Stratum protocol client

3. **Multi-Pool Support**
   - Automatic failover
   - Load balancing
   - Performance monitoring

4. **Historical Training**
   - Bitcoin historical data analysis
   - Neural network training
   - Pattern recognition

### 🔗 API Endpoints

Once deployed, access these endpoints at `https://YOUR_SERVICE_URL`:

- **Web Interface**: `/`
- **API Status**: `/api/status`
- **Mining Control**: `/api/real-mining/*`
- **Statistics**: `/api/stats`
- **WebSocket**: `/ws` (for real-time updates)

## Testing the Deployment

### 1. Check Service Health

```bash
SERVICE_URL=$(gcloud run services describe biomining-platform \
  --platform=managed \
  --region=us-central1 \
  --format="value(status.url)")

curl "$SERVICE_URL/api/status"
```

Expected response:
```json
{
  "status": "healthy",
  "version": "2.0.0",
  "features": {
    "bio_entropy": true,
    "real_mining": true,
    "historical_training": true
  }
}
```

### 2. Access Web Interface

Open your browser to the service URL:
```bash
echo "Web Interface: $SERVICE_URL"
```

### 3. Test Real Mining

Navigate to the "Real Bitcoin Mining" section and start a mining session.

## Monitoring

### View Logs

```bash
gcloud run logs read biomining-platform \
  --region=us-central1 \
  --limit=50
```

### Real-Time Logs

```bash
gcloud run logs tail biomining-platform \
  --region=us-central1
```

### Service Metrics

Visit the Cloud Run console:
```
https://console.cloud.google.com/run/detail/us-central1/biomining-platform/metrics
```

## Troubleshooting

### Build Failures

**Issue**: Docker build fails
**Solution**: Check Dockerfile syntax and verify all required files exist

```bash
# Verify required files
ls -la Dockerfile requirements.txt web/api/server.py
```

### Deployment Failures

**Issue**: Cloud Run deployment fails
**Solution**: Check logs and resource allocation

```bash
gcloud run logs read biomining-platform --region=us-central1 --limit=100
```

### Circular Import Errors

**Issue**: ImportError or circular import detected
**Solution**: This was fixed in PR #176 using lazy imports

The fix ensures:
- `BitcoinMiner` is imported lazily in `server.py`
- `PurePythonBioEntropyGenerator` is imported lazily in `real_bitcoin_miner.py`

### Memory Issues

**Issue**: Service runs out of memory
**Solution**: Increase memory allocation

```bash
gcloud run services update biomining-platform \
  --region=us-central1 \
  --memory 6Gi
```

## Cost Optimization

### Free Tier

Cloud Run offers a generous free tier:
- 2 million requests per month
- 360,000 GB-seconds of compute time
- 180,000 vCPU-seconds

### Cost Control

The deployment is configured with:
- **Max instances**: 3 (prevents runaway scaling)
- **Concurrency**: 10 (efficient resource usage)
- **Auto-scaling**: Scales to zero when not in use

### Estimated Costs

With 4Gi memory and 4 vCPU:
- **Active usage**: ~$0.048 per hour
- **Idle**: $0 (scales to zero)
- **Monthly (24/7)**: ~$35

For cost-effective testing, use:
- Smaller instance size (2Gi memory, 2 vCPU)
- Lower max instances (1-2)
- Testnet for mining operations

## Security

### Best Practices

1. **Authentication**: Enable Cloud Run authentication for production
2. **API Keys**: Use environment variables for sensitive data
3. **Network**: Configure VPC for internal services
4. **Secrets**: Use Secret Manager for credentials

### Enable Authentication

```bash
gcloud run services update biomining-platform \
  --region=us-central1 \
  --no-allow-unauthenticated
```

## Updates and Rollbacks

### Update Deployment

```bash
./deploy_python.sh
```

### Rollback to Previous Version

```bash
gcloud run services update-traffic biomining-platform \
  --region=us-central1 \
  --to-revisions=PREVIOUS_REVISION=100
```

### List Revisions

```bash
gcloud run revisions list \
  --service=biomining-platform \
  --region=us-central1
```

## Architecture

### Platform Components

```
┌─────────────────────────────────────┐
│     Google Cloud Run Service        │
│                                     │
│  ┌─────────────────────────────┐   │
│  │   FastAPI Server            │   │
│  │   (web/api/server.py)       │   │
│  └─────────────────────────────┘   │
│              ↓                      │
│  ┌─────────────────────────────┐   │
│  │   Bitcoin Miner Core        │   │
│  │   (real_bitcoin_miner.py)   │   │
│  └─────────────────────────────┘   │
│              ↓                      │
│  ┌─────────────────────────────┐   │
│  │   Stratum Client            │   │
│  │   (stratum_client.py)       │   │
│  └─────────────────────────────┘   │
│              ↓                      │
│  ┌─────────────────────────────┐   │
│  │   Bio-Entropy Generator     │   │
│  │   (Pure Python)             │   │
│  └─────────────────────────────┘   │
└─────────────────────────────────────┘
```

### Technology Stack

- **Runtime**: Python 3.11
- **Framework**: FastAPI
- **Protocol**: Stratum (Bitcoin mining)
- **Platform**: Google Cloud Run
- **Container**: Docker

## Support

### Issues

For deployment issues or bugs:
1. Check the logs first
2. Review troubleshooting section
3. Create an issue on GitHub with logs attached

### Documentation

- [Google Cloud Run Docs](https://cloud.google.com/run/docs)
- [Docker Documentation](https://docs.docker.com)
- [FastAPI Documentation](https://fastapi.tiangolo.com)

## Version History

### v2.0.0 (Current)
- ✅ Pure Python implementation
- ✅ Circular import fixes
- ✅ Real Bitcoin mining support
- ✅ Multi-pool support
- ✅ Bio-entropy integration

### v1.0.0
- Initial C++ implementation
- Basic mining functionality

---

**Ready to deploy!** 🚀

For questions or support, contact the development team.
